load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "smart_face_wasm",
    srcs = ["smart_face_main_wasm.cc"],
    linkopts = [
        "-sALLOW_MEMORY_GROWTH=1",
        "-sUSE_PTHREADS=0",
        "--bind",
        "-sTOTAL_STACK=128MB",
        "-sINITIAL_MEMORY=128MB",
        "-sEXPORT_NAME='createSmartFaceModule'",
        "-sMODULARIZE=1",
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
    ],
    data = [
        "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
    ],
    deps = [
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
    ],
)

# Extract WASM files for easier access
genrule(
    name = "extract_wasm",
    srcs = [":smart_face_wasm"],
    outs = [
        "output/smart_face_wasm.js",
        "output/smart_face_wasm.wasm",
    ],
    cmd = "tar -xf $(location :smart_face_wasm) -C $(@D)/output --strip-components=0 && " +
          "mkdir -p $(@D)/output && " +
          "tar -xf $(location :smart_face_wasm) -C $(@D)/output",
)

filegroup(
    name = "smart_capture_graph",
    srcs = ["smart_capture.pbtxt"],
)