cmake_minimum_required(VERSION 3.13)

# 导入 Pico SDK（如果 SDK 不在标准路径，可先设置 PICO_SDK_PATH）
# set(PICO_SDK_PATH /mnt/d/yangsen_files/pico-sdk)
include(pico_sdk_import.cmake)

# 设置目标板为 Pico W（必须在 project 之前）
set(PICO_BOARD pico_w)

# 定义项目名称和编程语言
project(pico_w_blink C CXX ASM)

# 初始化 SDK（必须）
pico_sdk_init()

# 在 pico_sdk_init() 之后添加
# message(STATUS "Pico SDK Version: ${PICO_SDK_VERSION_STRING}")

# 添加可执行文件
add_executable(pico_w_blink
    src/main.c
)

# 链接必要的库（注意只保留一个 cyw43 架构）
target_link_libraries(pico_w_blink
    pico_stdlib                # 包含基础库和 stdio
    pico_cyw43_arch_lwip_poll   # Pico W 无线架构（轮询模式）
)

# 在 target_link_libraries 之后添加
target_include_directories(pico_w_blink PRIVATE
    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/cyw43-driver/src
)

# 生成 UF2 等输出文件
pico_add_extra_outputs(pico_w_blink)

# 启用 USB 输出（用于 printf），禁用 UART（可选）
pico_enable_stdio_usb(pico_w_blink 1)
pico_enable_stdio_uart(pico_w_blink 0)