load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

cc_binary(
    # 🔥 ชื่อผลลัพธ์ที่ต้องการ (จะได้ไฟล์ smart_capture_wasm.js)
    name = "smart_capture_wasm",
    
    srcs = [],
    
    linkopts = [
        "-sALLOW_MEMORY_GROWTH=1",
        "-sINITIAL_MEMORY=32MB",
        "-sUSE_PTHREADS=0",
        "--bind",
        "-sTOTAL_STACK=16MB",
    ],
    
    # 📦 ฝังไฟล์ Graph และ Model ลงไปในระบบ
    data = [
        "smart_capture.pbtxt",    # ✅ ชื่อไฟล์จริงของคุณ (ไม่มี _wasm)
        "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
    ],
    
    deps = [
        "//mediapipe/calculators/smart_capture:smart_capture_calculator",
        "//mediapipe/graphs/face_mesh:desktop_live_calculators",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/port:status",
        "//mediapipe/modules/face_landmark:face_landmark_front_cpu",
    ],
)