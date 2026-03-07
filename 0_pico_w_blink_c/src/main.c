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