#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_pwm.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"
#include "sparkfun_nrf52840_mini.h"

#define LED NRF_GPIO_PIN_MAP(0,7)
#define BUTTON NRF_GPIO_PIN_MAP(0,13)

void GPIOTE_IRQHandler(){
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_IN_1);
    nrf_pwm_task_trigger(NRF_PWM0,NRF_PWM_TASK_NEXTSTEP);
}

void PWM0_IRQHandler(){
    nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);
    nrf_pwm_task_trigger(NRF_PWM0,NRF_PWM_TASK_SEQSTART0);
}

static void gpio_config(){
    nrf_gpiote_event_configure(1,BUTTON, NRF_GPIOTE_POLARITY_HITOLO);
    nrf_gpio_cfg_output(LED);
    nrf_gpio_pin_clear(LED);
    nrf_gpiote_int_enable(NRF_GPIOTE_INT_IN1_MASK);
    nrf_gpiote_event_enable(1);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

uint16_t comp1[5] = {2000,8000,12000,14000,15000}; 

static void pwm_config(){
    uint32_t pwm_pins[4] = {LED,NRF_PWM_PIN_NOT_CONNECTED,NRF_PWM_PIN_NOT_CONNECTED,NRF_PWM_PIN_NOT_CONNECTED};

    NVIC_EnableIRQ(PWM0_IRQn);

    nrf_pwm_pins_set(NRF_PWM0, pwm_pins);
    nrf_pwm_enable(NRF_PWM0);

    nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_8MHz,NRF_PWM_MODE_UP, 16000);
    nrf_pwm_seq_ptr_set(NRF_PWM0,0,comp1);
    nrf_pwm_seq_cnt_set(NRF_PWM0,0,5);
    nrf_pwm_loop_set(NRF_PWM0, 0);
    nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_TRIGGERED);
    
    nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);
    nrf_pwm_task_trigger(NRF_PWM0, NRF_PWM_TASK_SEQSTART0);
    nrf_pwm_int_enable(NRF_PWM0,NRF_PWM_INT_SEQEND0_MASK);
}


int main(void)
{   
    
    gpio_config();
    pwm_config();
    while(true){

        __WFE();
    
    }
  

}

