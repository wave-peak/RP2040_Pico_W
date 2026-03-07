## RP2040_Pico_W的C代码部署总结（依赖DeepSeek的强力AI辅助）
 - 1.开发工具相关
 - 2.编写调试代码
 - 3.编译代码 && 下载和烧录编译产物

## 开发环境相关
 - 1.windows的应用商店安装最新版的Ubuntu，例如：ubuntu 24.04.1 LTS
 - 2.安装必要的工具链
   - 添加 Kitware 的 APT 仓库来获取更新的 CMake 版本：
   - wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
   - sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ noble main'
   - sudo apt update

3.安装更新的 CMake
   - sudo apt install cmake

4.然后安装 ARM 工具链
   - sudo apt install gcc-arm-none-eabi
   - sudo apt install python3 python3-pip

5.安装完成后，验证工具链：
  - 检查 ARM GCC：arm-none-eabi-gcc --version
  - 检查 CMake：cmake --version
  - 检查 Python：python3 --version

6.获取Pico SDK
  - mkdir -p /mnt/e/my_project_rp2040
  - cd /mnt/e/my_project_rp2040
  - git clone -b master https://github.com/raspberrypi/pico-sdk.git
  - cd pico-sdk
  - git submodule update --init

7.设置环境变量 (添加到 ~/.bashrc 或 ~/.zshrc)
  - export PICO_SDK_PATH=/mnt/e/my_project_rp2040/pico-sdk
  - source ~/.bashrc
  - 确认PICO_SDK_PATH 已设置且正确：echo $PICO_SDK_PATH

## 创建项目结构
     my_pico_project/
     ├── CMakeLists.txt
     ├── pico_sdk_import.cmake
     └── src/
         └── main.c
 - 1.从Pico SDK复制pico_sdk_import.cmake文件到创建的项目文件夹的根目录
 - 2.进入项目根目录：cd my_project_rp2040
 - 3.检查是否有pico_sdk_import.cmake文件：ls -la
   - 如果没有的话，从Pico SDK 复制一份：cp $PICO_SDK_PATH/external/pico_sdk_import.cmake

4.CMakeLists.txt的配置信息
```cmake
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
#    hardware_pwm                # 如果需要 PWM
#    hardware_adc                # 如果需要 ADC
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
```

5.检查cyw43-driver子模块是否存在
  - 检查命令：ls -la $PICO_SDK_PATH/lib/cyw43-driver/
  - 如果不存在，到pico-sdk网站下载cyw43-driver压缩包解压到本地同样目录；网址：https://github.com/raspberrypi/pico-sdk/
  - 在Pico SDK目录下，查找cyw43_arch.h文件：find $PICO_SDK_PATH -name "cyw43_arch.h" | head -5

6.基础示例代码 (src/main.c)

```c
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
int main()
{
    // 初始化标准库
    stdio_init_all();
    
    // 初始化WiFi芯片
    if (cyw43_arch_init()) {
        printf("WiFi初始化失败\n");
        return -1;
    }
    
    // 设置LED引脚（Pico W板载LED）
    const uint LED_PIN = CYW43_WL_GPIO_LED_PIN;
    
    while (true) {
        // 点亮LED
        cyw43_arch_gpio_put(LED_PIN, 1);
        printf("LED ON\n");
        sleep_ms(500);
        
        // 熄灭LED
        cyw43_arch_gpio_put(LED_PIN, 0);
        printf("LED OFF\n");
        sleep_ms(500);
    }
    
    return 0;
}
```

## 创建构建目录并编译
 - cd my_pico_project/
 - mkdir build
 - rm -rf build/*
 - cd build

1.如果是编译普通的LED反转驱动，需要使用这个命令：
 - cmake ..

2.如果是编译cyw43-driver驱动，需要使用这个命令，指定板型为Pico W：
 - cmake -DPICO_BOARD=pico_w ..
 - make -j4

3.构建输出
构建完成后会生成以下文件：
 - my_pico_project.uf2 - 用于拖放编程
 - my_pico_project.elf - 用于调试
 - my_pico_project.bin - 二进制文件

## 部署到Pico W
1.UF2文件部署
 - 按住Pico W上的BOOTSEL按钮
 - 通过USB连接电脑
 - 释放BOOTSEL按钮
 - 将生成的.uf2文件拖放到出现的RPI-RP2驱动器
 - Pico W会自动重启并运行程序

2.使用picotool（命令行工具）部署
 安装picotool
 - git clone https://github.com/raspberrypi/picotool.git

 安装必要的 USB 库
 - sudo apt update
 - sudo apt install libusb-1.0-0-dev
 - cd picotool
 - mkdir build
 - cd build

 配置 CMake 启用 USB 支持
 - cmake -DPICOTOOL_USB=ON ..
 - make -j4
 - sudo cp picotool /usr/local/bin/
 - picotool version --verbose
 部署固件
 - picotool load my_pico_project.uf2

## 优化编译选项
在CMakeLists.txt中添加：
 - 1.优化级别
   - set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
 - 2.调试信息
   - set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

## 内存配置
1.根据项目需求调整内存分配在CMakeLists.txt中设置：
 - pico_set_binary_type(my_pico_project no_flash)

## 编译、链接
 - 通过main.c查看编译、链接的过程

## 编译pico-examples
1.下载pico-examples库
 - git clone git@github.com:raspberrypi/pico-examples.git

2.检查pico-sdk库下是否有cyw43-driver和lwip，检查命令如下：
 - ls -l $PICO_SDK_PATH/lib/cyw43-driver
 - ls -l $PICO_SDK_PATH/lib/lwip

 如果上面两个库都没有，需要进行下载：
 - git clone git@github.com:georgerobotics/cyw43-driver.git
 - git clone https://git.savannah.nongnu.org/git/lwip.git

3.编译pico-examples库
 - cd pico-examples/
 - mkdir build
 - rm -rf build/*
 - cd build

 如果是编译cyw43-driver驱动，需要使用这个命令，指定板型为Pico W：
 - cmake -DPICO_BOARD=pico_w ..
 - make -j4 > build.log
 