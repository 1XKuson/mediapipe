#include <cmath>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/calculators/smart_face/smart_capture_calculator.pb.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

namespace mediapipe {

namespace {
// Helper function to convert radians to degrees
float ToDegrees(float radians) {
  return radians * 180.0f / M_PI;
}
} // namespace

class SmartCaptureCalculator : public CalculatorBase {
 public:
  static absl::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
    // รับ FaceMesh Landmarks (468 จุด)
    cc->Inputs().Tag("LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
    
    // Output: ภาพที่ผ่านเกณฑ์และตัดแล้ว
    cc->Outputs().Tag("CROPPED_FACE").Set<ImageFrame>();
    // Output: (Optional) ส่งสถานะบอก JS ว่าผ่านหรือไม่ผ่านเพราะอะไร
    if (cc->Outputs().HasTag("STATUS")) {
       cc->Outputs().Tag("STATUS").Set<std::string>();
    }
    return absl::OkStatus();
  }

  absl::Status Open(CalculatorContext* cc) override {
    const auto& options = cc->Options<SmartCaptureCalculatorOptions>();
    max_captures_ = options.max_captures();
    max_yaw_ = options.max_yaw_degrees();
    max_pitch_ = options.max_pitch_degrees();
    padding_ = options.padding();
    current_count_ = 0;
    return absl::OkStatus();
  }

  absl::Status Process(CalculatorContext* cc) override {
    if (current_count_ >= max_captures_) return absl::OkStatus();
    if (cc->Inputs().Tag("IMAGE").IsEmpty() || cc->Inputs().Tag("LANDMARKS").IsEmpty()) return absl::OkStatus();

    const auto& input_img = cc->Inputs().Tag("IMAGE").Get<ImageFrame>();
    const auto& multi_landmarks = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();

    if (multi_landmarks.empty()) {
        SendStatus(cc, "No face detected");
        return absl::OkStatus();
    }

    const auto& landmarks = multi_landmarks[0];

    // --- 1. Geometric Logic Check (คำนวณมุม) ---
    // ใช้จุดอ้างอิง: จมูก(1), หูซ้าย(234), หูขวา(454)
    const auto& nose = landmarks.landmark(1);
    const auto& left_ear = landmarks.landmark(234);
    const auto& right_ear = landmarks.landmark(454);

    // คำนวณ Yaw (หันซ้ายขวา) จากความลึก Z ของหูเทียบกัน
    float yaw = ToDegrees(std::atan2(left_ear.z() - right_ear.z(), left_ear.x() - right_ear.x()));
    
    // คำนวณ Pitch (ก้มเงย) โดยประมาณจากตำแหน่งจมูกเทียบกับระดับหู
    float ear_mid_y = (left_ear.y() + right_ear.y()) / 2.0;
    float pitch = ToDegrees(std::atan2(nose.y() - ear_mid_y, nose.z()));
    // Note: การคำนวณ Pitch แบบนี้เป็นค่าประมาณ การคำนวณ 3D Pose จริงๆ ซับซ้อนกว่านี้มาก

    // ตรวจสอบเงื่อนไข
    if (std::abs(yaw) > max_yaw_) {
        SendStatus(cc, "Face turned too much (Yaw)");
        return absl::OkStatus();
    }
    // ปรับ threshold pitch ตามความเหมาะสมของโมเดลและมุมกล้อง
    if (std::abs(pitch) > max_pitch_ * 2.0) { 
         SendStatus(cc, "Face tilted up/down too much (Pitch)");
         return absl::OkStatus();
    }

    // --- 2. ผ่านเกณฑ์! ทำการ Crop ภาพ ---
    SendStatus(cc, "Captured!");
    CropAndSend(cc, input_img, landmarks);
    current_count_++;

    return absl::OkStatus();
  }

 private:
  void SendStatus(CalculatorContext* cc, std::string message) {
      if (cc->Outputs().HasTag("STATUS")) {
          cc->Outputs().Tag("STATUS").AddPacket(MakePacket<std::string>(message).At(cc->InputTimestamp()));
      }
  }

  void CropAndSend(CalculatorContext* cc, const ImageFrame& input_img, const NormalizedLandmarkList& landmarks) {
      cv::Mat src = formats::MatView(&input_img);
      int img_w = src.cols; int img_h = src.rows;

      float min_x = 1.0f, max_x = 0.0f, min_y = 1.0f, max_y = 0.0f;
      for (const auto& lm : landmarks.landmark()) {
          min_x = std::min(min_x, lm.x()); max_x = std::max(max_x, lm.x());
          min_y = std::min(min_y, lm.y()); max_y = std::max(max_y, lm.y());
      }

      int w = (max_x - min_x) * img_w; int h = (max_y - min_y) * img_h;
      int cx = (min_x * img_w) + w / 2; int cy = (min_y * img_h) + h / 2;
      int pad_w = w * (1.0f + padding_); int pad_h = h * (1.0f + padding_);
      int x = cx - pad_w / 2; int y = cy - pad_h / 2;

      x = std::max(0, x); y = std::max(0, y);
      pad_w = std::min(pad_w, img_w - x); pad_h = std::min(pad_h, img_h - y);

      if (pad_w > 0 && pad_h > 0) {
          cv::Rect roi(x, y, pad_w, pad_h);
          cv::Mat cropped; src(roi).copyTo(cropped);
          std::unique_ptr<ImageFrame> output_frame = absl::make_unique<ImageFrame>(
              input_img.Format(), cropped.cols, cropped.rows, ImageFrame::kDefaultAlignmentBoundary);
          cv::Mat output_mat = formats::MatView(output_frame.get());
          cropped.copyTo(output_mat);
          cc->Outputs().Tag("CROPPED_FACE").Add(output_frame.release(), cc->InputTimestamp());
      }
  }

  int max_captures_;
  float max_yaw_;
  float max_pitch_;
  float padding_;
  int current_count_;
};

REGISTER_CALCULATOR(SmartCaptureCalculator);
} // namespace mediapipe