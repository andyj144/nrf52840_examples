#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpiote.h"
#include "nrf_nvic.h"
#include "sparkfun_nrf52840_mini.h"

#define LED NRF_GPIO_PIN_MAP(0,7)
#define BUTTON NRF_GPIO_PIN_MAP(0,13)

void GPIOTE_IRQHandler(){
    nrf_gpiote_task_set(NRF_GPIOTE_TASKS_OUT_2);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_IN_1);
}

static void gpio_config(){
    nrf_gpiote_event_configure(1,BUTTON, NRF_GPIOTE_POLARITY_HITOLO);
    nrf_gpiote_task_configure(2,LED ,NRF_GPIOTE_POLARITY_TOGGLE,0);
    nrf_gpiote_int_enable(NRF_GPIOTE_INT_IN1_MASK);
    nrf_gpiote_event_enable(1);
    nrf_gpiote_task_enable(2);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    gpio_config();
    
    /* Toggle LEDs. */
    while (true)
    {
        __WFE();

    }
}

/**
 *@}
 **/
