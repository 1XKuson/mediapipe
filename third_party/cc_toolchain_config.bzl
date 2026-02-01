load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl", "tool_path")

def _impl(ctx):
    tool_paths = [
        tool_path(name = "gcc", path = "emcc"),
        tool_path(name = "ld", path = "emcc"),
        tool_path(name = "ar", path = "emar"),
        tool_path(name = "cpp", path = "emcc"),
        tool_path(name = "gcov", path = "/bin/false"),
        tool_path(name = "nm", path = "/bin/false"),
        tool_path(name = "objdump", path = "/bin/false"),
        tool_path(name = "strip", path = "/bin/false"),
    ]
    
    # เพิ่มส่วนนี้: บอก Bazel ว่าโฟลเดอร์เหล่านี้คือ System Headers
    builtin_include_dirs = [
        "/home/kuson/emsdk/upstream/emscripten/cache/sysroot",
        "/home/kuson/emsdk/upstream/lib/clang/23/include",
        "/home/kuson/emsdk/upstream/emscripten/system/include",
    ]
    
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = "wasm-toolchain",
        host_system_name = "i686-unknown-linux-gnu",
        target_system_name = "wasm32-unknown-emscripten",
        target_cpu = "wasm32",
        target_libc = "unknown",
        compiler = "emscripten",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
        cxx_builtin_include_directories = builtin_include_dirs, # <-- ส่งค่าเข้าไปตรงนี้
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)