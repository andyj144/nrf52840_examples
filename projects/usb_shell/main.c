#include <stdbool.h>
#include <stdint.h>
#include "nrf_clock.h"
#include "nrf_delay.h"
#include "nrf_timer.h"
#include "nrf_pwm.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"
#include "usb_task_api.h"
#include "shell_task.h"
#include "blinky_task.h"
#include "sparkfun_nrf52840_mini.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED NRF_GPIO_PIN_MAP(0,7)
#define BUTTON NRF_GPIO_PIN_MAP(0,13)


TaskHandle_t LED_task_handle;

void LED_task(void* task_parameters){
    while(true){    
        vTaskDelay(100);
        nrf_gpio_pin_toggle(LED);
        // taskYIELD();
    }
}


void GPIOTE_IRQHandler(){
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_IN_1);
    nrf_pwm_task_trigger(NRF_PWM0,NRF_PWM_TASK_NEXTSTEP);
}

void PWM0_IRQHandler(){
    nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);
    nrf_pwm_task_trigger(NRF_PWM0,NRF_PWM_TASK_SEQSTART0);
}

static void gpio_config(){
    // nrf_gpiote_event_configure(1,BUTTON, NRF_GPIOTE_POLARITY_HITOLO);
    nrf_gpio_cfg_output(LED);
    nrf_gpio_pin_clear(LED);
    // nrf_gpiote_int_enable(NRF_GPIOTE_INT_IN1_MASK);
    // nrf_gpiote_event_enable(1);
    // NVIC_EnableIRQ(GPIOTE_IRQn);
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

/**
 * @brief Function for application main entry.
 */
int main(void)
{   
    ret_code_t ret;

    

    
    // ret = NRF_LOG_INIT(NULL);
    // APP_ERROR_CHECK(ret);

    ret = nrf_drv_clock_init();
    // APP_ERROR_CHECK(ret);
    
    nrf_drv_clock_lfclk_request(NULL);

    while(!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting */
    }

    // ret = app_timer_init();
    // APP_ERROR_CHECK(ret);

//     init_bsp();
// #if NRF_CLI_ENABLED
//     init_cli();
// #endif

 
    
    
    // APP_ERROR_CHECK(ret);
    add_shell_command(&blinky_cmd);
    ret = xTaskCreate(blinky_task,"BLNK", 1000, NULL, 2, &blinky_task_handle);
    ret = xTaskCreate(USB_task,"USB1", 1000, NULL,2, &USB_task_handle);
    ret = xTaskCreate(shell_task,"shl1", 1000, NULL, 1, &shell_task_handle);

    vTaskStartScheduler();

    while(true){

    }
  

}

/**
 *@}
 **/
