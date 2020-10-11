#include <stdbool.h>
#include <stdint.h>
#include "nrf_drv_clock.h"
// #include "nrf_delay.h"
// #include "nrf_timer.h"
// #include "nrf_pwm.h"
// #include "nrf_gpiote.h"
// #include "nrf_gpio.h"
// #include "nrf_nvic.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "sparkfun_nrf52840_mini.h"


static void cdc_acm_user_evt_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define LED NRF_GPIO_PIN_MAP(0,7)
#define BUTTON NRF_GPIO_PIN_MAP(0,13)

#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION false
#endif

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_evt_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

#define READ_SIZE 1

static char pc_rx_buffer[READ_SIZE];
static char pc_tx_buffer[NRF_DRV_USBD_EPSIZE];

/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_evt_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Watch for a transfer.  If you don't start with this, the RX_DONE event will never happen.*/
            ret_code_t ret = app_usbd_cdc_acm_read(p_cdc_acm,
                                                   pc_rx_buffer,
                                                   READ_SIZE);
            UNUSED_VARIABLE(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            do
            {
                ret = app_usbd_cdc_acm_read(p_cdc_acm, pc_rx_buffer, READ_SIZE);
                pc_tx_buffer[0] = pc_rx_buffer[0];
                app_usbd_cdc_acm_write(p_cdc_acm,
                                        pc_tx_buffer,
                                        READ_SIZE);
            } while (ret == NRF_SUCCESS);

            break;
        }
        default:
            break;
    }
}

static void usbd_user_evt_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            app_usbd_start();
            break;
        default:
            break;
    }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{   

    ret_code_t ret;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_evt_handler
    };

    ret = nrf_drv_clock_init();
    
    nrf_drv_clock_lfclk_request(NULL);

    while(!nrf_drv_clock_lfclk_is_running())
    {
        
    }
    
    app_usbd_serial_num_generate();
    
    ret = app_usbd_init(&usbd_config);


    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    UNUSED_VARIABLE(ret);
    
    if (USBD_POWER_DETECTION)
    {
        
        ret = app_usbd_power_events_enable();
      
    }
    else
    {
        app_usbd_enable();
        app_usbd_start();
    }
  
    while (true)
    {
        while (app_usbd_event_queue_process())
        {
            
        }
        /* Sleep CPU only if there was no interrupt since last loop processing */
        __WFE();
    }
}