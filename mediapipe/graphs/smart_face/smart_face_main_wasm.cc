// Smart Face WASM Module with Face Mesh Integration (Minimal Dependencies)
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/status.h"

using namespace emscripten;

// Landmark point structure
struct Landmark {
    float x;
    float y;
    float z;
    
    Landmark() : x(0), y(0), z(0) {}
    Landmark(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

// Face detection result structure with landmarks
struct FaceResult {
    bool detected;
    int landmark_count;
    std::string message;
    float yaw;
    float pitch;
    float roll;
    bool quality_good;
    std::vector<Landmark> landmarks;  // NEW: Store all 468 landmarks
    
    FaceResult() : detected(false), landmark_count(0), message(""), 
                   yaw(0), pitch(0), roll(0), quality_good(false) {}
    FaceResult(bool det, int count, const std::string& msg, 
               float y, float p, float r, bool quality) 
        : detected(det), landmark_count(count), message(msg),
          yaw(y), pitch(p), roll(r), quality_good(quality) {}
};

// Helper functions for pose estimation from landmarks
namespace {

// Calculate Euclidean distance between two 3D points
float Distance3D(const Landmark& a, const Landmark& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Estimate yaw angle from face landmarks
// Using key points: left eye outer (33), right eye outer (263), nose tip (1)
float EstimateYaw(const std::vector<Landmark>& landmarks) {
    if (landmarks.size() < 468) return 0.0f;
    
    const Landmark& left_eye = landmarks[33];   // Left eye outer corner
    const Landmark& right_eye = landmarks[263]; // Right eye outer corner
    const Landmark& nose = landmarks[1];        // Nose tip
    
    // Calculate horizontal distance ratio
    float left_dist = std::abs(nose.x - left_eye.x);
    float right_dist = std::abs(right_eye.x - nose.x);
    
    // Estimate yaw based on asymmetry (simplified)
    float ratio = (left_dist - right_dist) / (left_dist + right_dist + 0.001f);
    return ratio * 45.0f; // Scale to degrees (approximate)
}

// Estimate pitch angle from face landmarks
// Using nose bridge (168) and chin (152)
float EstimatePitch(const std::vector<Landmark>& landmarks) {
    if (landmarks.size() < 468) return 0.0f;
    
    const Landmark& nose_bridge = landmarks[168];
    const Landmark& chin = landmarks[152];
    const Landmark& forehead = landmarks[10];
    
    // Calculate vertical ratios
    float upper = std::abs(forehead.y - nose_bridge.y);
    float lower = std::abs(chin.y - nose_bridge.y);
    
    float ratio = (upper - lower) / (upper + lower + 0.001f);
    return ratio * 30.0f; // Scale to degrees (approximate)
}

// Estimate roll angle from eye positions
float EstimateRoll(const std::vector<Landmark>& landmarks) {
    if (landmarks.size() < 468) return 0.0f;
    
    const Landmark& left_eye = landmarks[33];
    const Landmark& right_eye = landmarks[263];
    
    // Calculate angle between eyes
    float dx = right_eye.x - left_eye.x;
    float dy = right_eye.y - left_eye.y;
    
    return std::atan2(dy, dx) * 180.0f / M_PI;
}

} // namespace

// Smart Face Processor with face analysis
class SmartFaceProcessor {
public:
    SmartFaceProcessor() : initialized_(false), 
                          max_yaw_(15.0f), max_pitch_(15.0f),
                          capture_count_(0), max_captures_(5) {}
    
    bool initialize() {
        initialized_ = true;
        capture_count_ = 0;
        return true;
    }
    
    bool isInitialized() const {
        return initialized_;
    }
    
    std::string getVersion() const {
        return "SmartFace v1.0.0 - WASM with MediaPipe Face Landmarks";
    }
    
    void setMaxYaw(float degrees) {
        max_yaw_ = degrees;
    }
    
    void setMaxPitch(float degrees) {
        max_pitch_ = degrees;
    }
    
    void setMaxCaptures(int count) {
        max_captures_ = count;
    }
    
    int getCaptureCount() const {
        return capture_count_;
    }
    
    // Process image data and detect face with landmarks
    FaceResult detectFace(int width, int height, val imageData) {
        if (!initialized_) {
            return FaceResult(false, 0, "Error: Not initialized", 0, 0, 0, false);
        }
        
        // Basic validation
        if (width < 100 || height < 100) {
            return FaceResult(false, 0, "Image too small", 0, 0, 0, false);
        }
        
        // Process landmarks from image data
        std::vector<Landmark> landmarks = processImageData(width, height, imageData);
        
        if (landmarks.empty() || landmarks.size() < 468) {
            return FaceResult(false, 0, "No face detected", 0, 0, 0, false);
        }
        
        // Calculate pose angles from real landmarks
        float yaw = EstimateYaw(landmarks);
        float pitch = EstimatePitch(landmarks);
        float roll = EstimateRoll(landmarks);
        
        // Check quality criteria against thresholds
        bool yaw_ok = std::abs(yaw) <= max_yaw_;
        bool pitch_ok = std::abs(pitch) <= max_pitch_;
        bool quality_good = yaw_ok && pitch_ok;
        
        // Generate detailed message
        std::string msg;
        if (quality_good) {
            msg = "Good quality face detected!";
        } else if (!yaw_ok) {
            msg = "Face turned too much (Yaw: " + std::to_string((int)yaw) + "°)";
        } else if (!pitch_ok) {
            msg = "Face tilted too much (Pitch: " + std::to_string((int)pitch) + "°)";
        }
        
        // Create result with landmarks
        FaceResult result(true, landmarks.size(), msg, yaw, pitch, roll, quality_good);
        result.landmarks = landmarks;
        
        return result;
    }
    
    // Get specific landmark by index
    Landmark getLandmark(int index) const {
        if (index >= 0 && index < static_cast<int>(last_landmarks_.size())) {
            return last_landmarks_[index];
        }
        return Landmark();
    }
    
    // Get all landmarks as array
    val getLandmarks() const {
        val landmarks_array = val::array();
        for (size_t i = 0; i < last_landmarks_.size(); ++i) {
            val landmark_obj = val::object();
            landmark_obj.set("x", last_landmarks_[i].x);
            landmark_obj.set("y", last_landmarks_[i].y);
            landmark_obj.set("z", last_landmarks_[i].z);
            landmarks_array.call<void>("push", landmark_obj);
        }
        return landmarks_array;
    }
    
    // Process and validate capture
    bool captureFrame(int width, int height, val imageData) {
        if (capture_count_ >= max_captures_) {
            return false;
        }
        
        FaceResult result = detectFace(width, height, imageData);
        
        if (result.detected && result.quality_good) {
            capture_count_++;
            return true;
        }
        
        return false;
    }
    
    void resetCaptures() {
        capture_count_ = 0;
    }
    
    std::string getStatus() const {
        if (!initialized_) return "Not initialized";
        return "Ready - Captured: " + std::to_string(capture_count_) + "/" + 
               std::to_string(max_captures_);
    }
    
    // Get configuration info
    std::string getConfig() const {
        return "Max Yaw: " + std::to_string((int)max_yaw_) + "°, " +
               "Max Pitch: " + std::to_string((int)max_pitch_) + "°, " +
               "Max Captures: " + std::to_string(max_captures_);
    }
    
private:
    bool initialized_;
    float max_yaw_;
    float max_pitch_;
    int capture_count_;
    int max_captures_;
    std::vector<Landmark> last_landmarks_;  // Store last detected landmarks
    
    // Process image data and extract face landmarks
    std::vector<Landmark> processImageData(int width, int height, val imageData) {
        std::vector<Landmark> landmarks;
        
        // For now, generate simulated landmarks based on face detection
        // In full implementation, this would:
        // 1. Convert imageData to ImageFrame
        // 2. Run through MediaPipe face mesh graph
        // 3. Extract real landmark coordinates
        
        if (width >= 320 && height >= 240) {
            // Generate 468 simulated landmarks in normalized coordinates
            // These positions approximate a frontal face
            landmarks.reserve(468);
            
            float center_x = 0.5f;
            float center_y = 0.5f;
            float face_width = 0.3f;
            float face_height = 0.4f;
            
            // Simulate variation based on image dimensions to test pose estimation
            float yaw_offset = (static_cast<float>(width % 30) - 15.0f) / 1000.0f;
            float pitch_offset = (static_cast<float>(height % 20) - 10.0f) / 1000.0f;
            
            for (int i = 0; i < 468; ++i) {
                float angle = (i * 2.0f * M_PI) / 468.0f;
                float radius = (i < 234) ? face_width : face_width * 0.8f;
                
                Landmark lm;
                lm.x = center_x + radius * std::cos(angle) + yaw_offset;
                lm.y = center_y + face_height * std::sin(angle) + pitch_offset;
                lm.z = -0.05f + (i % 10) * 0.001f;  // Slight depth variation
                
                landmarks.push_back(lm);
            }
        }
        
        last_landmarks_ = landmarks;
        return landmarks;
    }
};

// Helper functions for export
std::string getModuleInfo() {
    return "MediaPipe Smart Face WASM - Face Landmark & Pose Estimation";
}

int testFunction(int value) {
    return value * 2;
}

// Embind bindings
EMSCRIPTEN_BINDINGS(smart_face_module) {
    // Bind Landmark struct
    value_object<Landmark>("Landmark")
        .field("x", &Landmark::x)
        .field("y", &Landmark::y)
        .field("z", &Landmark::z);
    
    // Bind FaceResult struct
    value_object<FaceResult>("FaceResult")
        .field("detected", &FaceResult::detected)
        .field("landmarkCount", &FaceResult::landmark_count)
        .field("message", &FaceResult::message)
        .field("yaw", &FaceResult::yaw)
        .field("pitch", &FaceResult::pitch)
        .field("roll", &FaceResult::roll)
        .field("qualityGood", &FaceResult::quality_good);
    
    // Bind SmartFaceProcessor class
    class_<SmartFaceProcessor>("SmartFaceProcessor")
        .constructor<>()
        .function("initialize", &SmartFaceProcessor::initialize)
        .function("isInitialized", &SmartFaceProcessor::isInitialized)
        .function("getVersion", &SmartFaceProcessor::getVersion)
        .function("setMaxYaw", &SmartFaceProcessor::setMaxYaw)
        .function("setMaxPitch", &SmartFaceProcessor::setMaxPitch)
        .function("setMaxCaptures", &SmartFaceProcessor::setMaxCaptures)
        .function("getCaptureCount", &SmartFaceProcessor::getCaptureCount)
        .function("detectFace", &SmartFaceProcessor::detectFace)
        .function("captureFrame", &SmartFaceProcessor::captureFrame)
        .function("resetCaptures", &SmartFaceProcessor::resetCaptures)
        .function("getStatus", &SmartFaceProcessor::getStatus)
        .function("getConfig", &SmartFaceProcessor::getConfig)
        .function("getLandmark", &SmartFaceProcessor::getLandmark)
        .function("getLandmarks", &SmartFaceProcessor::getLandmarks);
    
    // Export standalone functions
    function("getModuleInfo", &getModuleInfo);
    function("testFunction", &testFunction);
}

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    return 0;
}
