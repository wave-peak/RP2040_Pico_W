#RP2040_Pico_W的C代码部署总结（依赖DeepSeek的强力AI辅助）
1.开发工具相关
2.编写调试代码
3.编译代码 && 下载和烧录编译产物

#开发环境相关
1.windows的应用商店安装最新版的Ubuntu LTS

1.1 安装必要的工具链
# 添加 Kitware 的 APT 仓库来获取更新的 CMake 版本
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ noble main'
sudo apt update

# 安装更新的 CMake
sudo apt install cmake

# 然后安装 ARM 工具链
sudo apt install gcc-arm-none-eabi
sudo apt install python3 python3-pip
验证安装
安装完成后，验证工具链：

# 检查 ARM GCC
arm-none-eabi-gcc --version
# 检查 CMake
cmake --version
# 检查 Python
python3 --version

2.获取Pico SDK
mkdir -p /mnt/e/my_project_rp2040
cd /mnt/e/my_project_rp2040
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

# 设置环境变量 (添加到 ~/.bashrc 或 ~/.zshrc)
export PICO_SDK_PATH=/mnt/e/my_project_rp2040/pico-sdk
source ~/.bashrc
# 确认 PICO_SDK_PATH 已设置且正确
echo $PICO_SDK_PATH

3.创建项目结构
my_pico_project/
├── CMakeLists.txt
├── pico_sdk_import.cmake
└── src/
    └── main.c
3.0 pico_sdk_import.cmake的内容需要从Pico SDK中复制过来
# 1. 进入你的项目目录
cd my_project_rp2040
# 2. 检查是否有 pico_sdk_import.cmake 文件
ls -la
# 3. 如果没有，从 Pico SDK 复制一份
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake

3.1 CMakeLists.txt配置
cmake_minimum_required(VERSION 3.13)

# 导入Pico SDK
include(pico_sdk_import.cmake)

# 初始化项目
project(pico_project C CXX ASM)

# 设置目标板为 Pico W
set(PICO_BOARD pico_w)

# 初始化Pico SDK
pico_sdk_init()

# 添加可执行文件
add_executable(pico_project 
    src/main_led.c
)

target_link_libraries(pico_project
    pico_stdlib
    hardware_gpio
    hardware_uart
    hardware_i2c
    hardware_spi
    hardware_pwm
    hardware_adc
    hardware_flash
)

# 对于Pico W，需要添加WiFi相关的库
target_link_libraries(pico_project
    pico_cyw43_arch_none
)

# 启用USB输出（可选）
pico_enable_stdio_usb(pico_project 1)
pico_enable_stdio_uart(pico_project 0)

# 生成必要的文件
pico_add_extra_outputs(pico_project)

#检查cyw43-driver子模块是否存在
ls -la $PICO_SDK_PATH/lib/cyw43-driver/
#如果不存在下载压缩包解压到此目录 or 其他方法下载到此目录
#在Pico SDK目录下，查找cyw43_arch.h文件：
find $PICO_SDK_PATH -name "cyw43_arch.h" | head -5

3.2 基础示例代码 (src/main.c)
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

int main() {
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

3. 构建项目
3.1 创建构建目录并编译

mkdir build
rm -rf build/*
cd build
#如果是编译普通的LED反转驱动，需要使用这个命令：
cmake ..
#如果是编译cyw43-driver驱动，需要使用这个命令，指定板型为Pico W：
cmake -DPICO_BOARD=pico_w ..
make -j4

3.2 构建输出
构建完成后会生成以下文件：
my_pico_project.uf2 - 用于拖放编程
my_pico_project.elf - 用于调试
my_pico_project.bin - 二进制文件

4. 部署到Pico W
4.1 UF2文件部署
按住Pico W上的BOOTSEL按钮
通过USB连接电脑
释放BOOTSEL按钮
将生成的.uf2文件拖放到出现的RPI-RP2驱动器
Pico W会自动重启并运行程序

4.2 使用picotool（命令行工具）
bash
# 安装picotool
cd pico-sdk/tools/picotool
mkdir build
cd build
cmake ..
make
sudo cp picotool /usr/local/bin/

# 部署固件
picotool load my_pico_project.uf2


4.3 优化编译选项
在CMakeLists.txt中添加：
# 优化级别
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
# 调试信息
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

4.4 内存配置
根据项目需求调整内存分配：
# 在CMakeLists.txt中设置
pico_set_binary_type(my_pico_project no_flash)
