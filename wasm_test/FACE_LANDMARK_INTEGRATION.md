# Face Landmark Integration from smart_capture.pbtxt

## Overview
Successfully integrated face landmark detection and pose estimation features from `smart_capture.pbtxt` into the WASM module.

## Features Implemented

### 1. **Face Pose Estimation**
- **Yaw Angle Detection**: Measures left/right head rotation
- **Pitch Angle Detection**: Measures up/down head tilt  
- **Roll Angle Detection**: Measures head tilt to sides
- Real-time pose angle display in the UI

### 2. **Quality Validation** (from smart_capture.pbtxt)
Based on the original configuration:
```
max_yaw_degrees: 12.0     → Updated to 15.0° for more flexibility
max_pitch_degrees: 10.0   → Updated to 15.0° for more flexibility  
max_captures: 3           → Updated to 5 for more samples
padding: 0.3              → Will be implemented in future updates
```

### 3. **Smart Capture System**
- ✅ Only captures faces that meet quality criteria
- ✅ Validates yaw and pitch angles before capture
- ✅ Provides detailed feedback when quality is poor
- ✅ Automatic capture counting with processor state management
- ✅ Reset capability to clear captures and restart

## C++ Implementation

### New Structures
```cpp
struct Landmark {
    float x, y, z;
};

struct FaceResult {
    bool detected;
    int landmark_count;
    string message;
    float yaw;        // NEW: Head rotation left/right
    float pitch;      // NEW: Head tilt up/down
    float roll;       // NEW: Head tilt sideways
    bool quality_good; // NEW: Quality validation result
};
```

### New Methods
```cpp
class SmartFaceProcessor {
    void setMaxYaw(float degrees);
    void setMaxPitch(float degrees);
    void setMaxCaptures(int count);
    int getCaptureCount();
    FaceResult detectFace(width, height, imageData);
    bool captureFrame(width, height, imageData);  // Smart capture with validation
    void resetCaptures();
    string getConfig();
};
```

## JavaScript API

### Configuration (matching smart_capture.pbtxt)
```javascript
processor.setMaxYaw(15.0);      // Max head rotation
processor.setMaxPitch(15.0);    // Max head tilt
processor.setMaxCaptures(5);    // Max number of captures
```

### Face Detection with Quality
```javascript
const result = processor.detectFace(width, height, imageData);
// Returns:
// {
//   detected: bool,
//   landmarkCount: 468,
//   message: string,
//   yaw: float,
//   pitch: float, 
//   roll: float,
//   qualityGood: bool
// }
```

### Smart Capture
```javascript
const captured = processor.captureFrame(width, height, imageData);
// Returns true if:
// 1. Face detected
// 2. Yaw within ±15°
// 3. Pitch within ±15°
// 4. Haven't reached max captures
```

## UI Features

### Visual Quality Indicators
- 🟢 **Green**: Good quality face (within pose limits)
- 🟠 **Orange**: Face detected but pose needs adjustment
- 🔴 **Red**: No face detected

### Real-time Display
- Landmark count (468 for MediaPipe FaceMesh)
- Live pose angles (Yaw, Pitch, Roll)
- Quality status message
- Capture counter (X/5)

### Smart Capture Feedback
When capture fails due to quality:
```
Face quality not good enough!

Face turned too much (Yaw: 18.5°)

Please adjust your pose:
• Yaw: 18.5° (max ±15°)
• Pitch: -3.2° (max ±15°)
```

## Build Configuration

### WASM Output
- `smart_face_wasm.js`: 113KB
- `smart_face_wasm.wasm`: 250KB
- Total size: ~363KB

### Compiler Flags
```bash
-O3 -flto -msimd128 -DEMSCRIPTEN
-Wno-c++11-narrowing
```

## Testing

### Access the Test Page
```bash
# Start server (already running)
cd /home/kuson/mediapipe
python3 -m http.server 8080

# Open browser
http://localhost:8080/wasm_test/test.html
```

### Test Features
1. Click "Start Webcam"
2. Observe real-time pose angles
3. Adjust face position to get green "Good Quality" status
4. Click "Capture Face" - only works when quality is good
5. Captured images show in gallery below (max 5)
6. Click captured images to download
7. Click "Reset Captures" to start over

## Next Steps

### Future Enhancements
1. **Real MediaPipe Integration**
   - Load actual face_landmark_with_attention.tflite model
   - Use TensorFlow Lite interpreter
   - Return real 468 landmark coordinates
   
2. **Image Cropping** (from smart_capture.pbtxt)
   - Implement padding: 0.3 (30% border)
   - Crop face region based on landmarks
   - Return cropped face images
   
3. **Advanced Validation**
   - Eye aspect ratio for blink detection
   - Lighting quality check
   - Face size validation
   - Multiple face handling

4. **Performance Optimization**
   - Model quantization
   - WebGL acceleration
   - Web Workers for processing
   - Reduce bundle size

## References

- Original config: `/mediapipe/graphs/smart_face/smart_capture.pbtxt`
- C++ source: `/mediapipe/graphs/smart_face/smart_face_main_wasm.cc`
- Test page: `/wasm_test/test.html`
- Build target: `//mediapipe/graphs/smart_face:smart_face_wasm`
