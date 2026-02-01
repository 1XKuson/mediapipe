# Emscripten OpenCV
# Headers from OpenCV source, linking via Emscripten's port system
package(default_visibility = ["//visibility:public"])

# Generate opencv_modules.hpp
genrule(
    name = "opencv_modules_hpp",
    outs = ["opencv2/opencv_modules.hpp"],
    cmd = """
cat > $@ <<'EOF'
#define HAVE_OPENCV_CORE
#define HAVE_OPENCV_IMGPROC
#define HAVE_OPENCV_IMGCODECS
#define HAVE_OPENCV_VIDEOIO
#define HAVE_OPENCV_HIGHGUI
#define HAVE_OPENCV_VIDEO
#define HAVE_OPENCV_CALIB3D
#define HAVE_OPENCV_FEATURES2D
#define HAVE_OPENCV_OBJDETECT
#define HAVE_OPENCV_DNN
#define HAVE_OPENCV_FLANN
#define HAVE_OPENCV_PHOTO
EOF
""",
)

# Generate cvconfig.h
genrule(
    name = "cvconfig_h",
    outs = ["cvconfig.h"],
    cmd = """
cat > $@ <<'EOF'
#ifndef OPENCV_CVCONFIG_H_INCLUDED
#define OPENCV_CVCONFIG_H_INCLUDED
#define CV_INLINE static inline
#endif
EOF
""",
)

# Generate cv_cpu_config.h
genrule(
    name = "cv_cpu_config_h",
    outs = ["cv_cpu_config.h"],
    cmd = """
cat > $@ <<'EOF'
#ifndef OPENCV_CV_CPU_CONFIG_H_INCLUDED
#define OPENCV_CV_CPU_CONFIG_H_INCLUDED
#endif
EOF
""",
)

# Generate opencv_data_config.hpp
genrule(
    name = "opencv_data_config_hpp",
    outs = ["opencv2/opencv_data_config.hpp"],
    cmd = """
cat > $@ <<'EOF'
#ifndef OPENCV_DATA_CONFIG_HPP
#define OPENCV_DATA_CONFIG_HPP
#endif
EOF
""",
)

# Generate opencv2/core/cvstd.hpp stub
genrule(
    name = "cvstd_hpp",
    outs = ["opencv2/core/cvstd.hpp"],
    cmd = """
cat > $@ <<'EOF'
#ifndef OPENCV_CORE_CVSTD_HPP
#define OPENCV_CORE_CVSTD_HPP
#include <string>
#include <vector>
#include <memory>
namespace cv {
  typedef std::string String;
  template<typename T> using Ptr = std::shared_ptr<T>;
  template<typename T> struct DefaultDeleter {
    void operator()(T* obj) const { delete obj; }
  };
}
#endif
EOF
""",
)

# Minimal OpenCV stub library for WASM
# Provides headers but minimal/stub implementation
cc_library(
    name = "opencv",
    srcs = ["opencv_wasm_stub.cpp"],
    hdrs = glob([
        "modules/core/include/**/*.h*",
        "modules/imgproc/include/**/*.h*",
    ]) + [
        ":opencv_modules_hpp",
        ":cvconfig_h",
        ":cv_cpu_config_h",
        ":opencv_data_config_hpp",
        ":cvstd_hpp",
    ],
    includes = [
        ".",
        "modules/core/include",
        "modules/imgproc/include",
    ],
)

# Create a minimal stub implementation file
genrule(
    name = "opencv_wasm_stub_cpp",
    outs = ["opencv_wasm_stub.cpp"],
    cmd = """
cat > $@ <<'EOF'
// Minimal OpenCV stub for WASM builds
// This provides just enough to compile but may need full implementation
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// Note: This is a stub - actual OpenCV functionality needs to be implemented
// or image processing should be handled differently in WASM

namespace cv {
// Add minimal stub implementations as needed
}
EOF
""",
)
