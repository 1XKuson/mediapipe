package(default_visibility = ["//visibility:public"])
load("@//third_party:cc_toolchain_config.bzl", "cc_toolchain_config")

filegroup(name = "everything", srcs = glob(["**/*"]))

cc_toolchain_suite(
    name = "toolchain",
    toolchains = { "wasm32": ":wasm_toolchain" },
)

filegroup(name = "empty")

cc_toolchain(
    name = "wasm_toolchain",
    toolchain_identifier = "wasm-toolchain",
    toolchain_config = ":wasm_toolchain_config",
    all_files = ":everything",
    compiler_files = ":everything",
    dwp_files = ":empty",
    linker_files = ":everything",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
)

cc_toolchain_config(name = "wasm_toolchain_config")