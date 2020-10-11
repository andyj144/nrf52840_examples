#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "sparkfun_nrf52840_mini.h"

#define LED NRF_GPIO_PIN_MAP(0,7)

static void gpio_config(){
    nrf_gpio_cfg_output(LED);
    nrf_gpio_pin_set(LED);
}


int main(void)
{
    gpio_config();
    while (true)
    {
        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED);
    }
}
