# ARM Cortex-M 交叉编译工具链（arm-none-eabi）
# 供 CMakeLists.txt 通过 CMAKE_TOOLCHAIN_FILE 引用，也可在命令行显式指定：
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -B build

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# 交叉编译器
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR           arm-none-eabi-ar)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)
set(SIZE               arm-none-eabi-size)

# 目标板没有操作系统，编译期无法链接可执行文件，
# 让 try_compile 只生成静态库，否则 CMake 的编译器探测会失败。
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
