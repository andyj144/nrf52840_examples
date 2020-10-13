#include "usb_task.h"
#include "usb_task_api.h"

TaskHandle_t USB_task_handle;

QueueHandle_t USB_task_api_q_handle;

QueueHandle_t USB_CDC_rcv_q_handle;
QueueHandle_t USB_CDC_send_q_handle;

QueueHandle_t task_started;
bool _task_started = true;


SemaphoreHandle_t USB_CDC_rcv_mutex;

SemaphoreHandle_t USB_CDC_send_mutex;




static char m_rx_buffer[READ_SIZE];
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];
static char tx_char_buffer;
static char *tx_string_ptr;
static SemaphoreHandle_t tx_string_ownership;
static int tx_string_len;
static int tx_string_unsent;

static int tx_head=0;
static int tx_tail=0;
static bool tx_buffer_full=false;
static bool tx_buffer_empty=true;
static bool tx_pending = false;

static bool m_send_flag = 0;

static bool rx_empty = true;

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_event_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);


void usb_new_event_isr_handler(app_usbd_internal_evt_t const * const event, bool queued)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    usb_task_api_msg_t api_message;
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(queued);
    ASSERT(USB_task_handle != NULL);

    /* Release the semaphore */
    api_message.msg_type = USB_TASK_INT;
    api_message.data = NULL;

    xQueueSendFromISR(USB_task_api_q_handle,&api_message,&xHigherPriorityTaskWoken);
    // vTaskNotifyGiveFromISR(USB_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_event_handler(app_usbd_class_inst_t const * inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(inst);
    bool buffer_was_full;

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            // bsp_board_led_on(LED_CDC_ACM_OPEN);

            /*Setup first transfer*/
            const char *first_out = "Always be Cobbling";
            

            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   READ_SIZE);

        
            UNUSED_VARIABLE(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            // bsp_board_led_off(LED_CDC_ACM_OPEN);
            break;
            
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            tx_pending = false;
            if(tx_string_unsent > 0){
                if (tx_string_unsent > NRFX_USBD_EPSIZE){
                    tx_string_unsent = tx_string_unsent - NRF_DRV_USBD_EPSIZE;
                    app_usbd_cdc_acm_write(&m_app_cdc_acm, tx_string_ptr, NRF_DRV_USBD_EPSIZE);
                }
                else{
                    app_usbd_cdc_acm_write(&m_app_cdc_acm, tx_string_ptr, NRF_DRV_USBD_EPSIZE);
                    tx_string_unsent = 0;
                }
            }
            else{
                if(tx_string_ownership){
                    xSemaphoreGive(tx_string_ownership);
                }
                usb_send_queue();
            }
            break;

        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            rx_empty = false;
            rx_done_handler();
            // bsp_board_led_invert(LED_CDC_ACM_RX);
            break;
        }
        default:
            break;
    }
}

static void usbd_user_event_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            // bsp_board_led_off(LED_USB_RESUME);
            break;
        case APP_USBD_EVT_DRV_RESUME:
            // bsp_board_led_on(LED_USB_RESUME);
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            // bsp_board_leds_off();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            // NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            // NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            // NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}


void USB_task(void * task_parameters){
    ret_code_t ret;
    usb_task_api_msg_t api_message;
    tx_head = 0;
    tx_tail = 0;
    tx_string_ownership = NULL;

    USB_task_api_q_handle = xQueueCreate(10,sizeof(usb_task_api_msg_t));
    USB_CDC_rcv_q_handle = xQueueCreate(20,sizeof(com_data_t));
    USB_CDC_send_q_handle = xQueueCreate(20,sizeof(com_data_t));

    task_started = xQueueCreate(1,sizeof(bool));

    USB_CDC_send_mutex = xSemaphoreCreateRecursiveMutex();
    USB_CDC_rcv_mutex = xSemaphoreCreateRecursiveMutex();

    static const app_usbd_config_t usbd_config = {
        .ev_isr_handler = usb_new_event_isr_handler,
        .ev_state_proc = usbd_user_event_handler
    };

    app_usbd_serial_num_generate();
    
    ret = app_usbd_init(&usbd_config);
    
    // APP_ERROR_CHECK(ret);
    // NRF_LOG_INFO("USBD CDC ACM example started.");

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);

    if (USBD_POWER_DETECTION)
    {
        
        ret = app_usbd_power_events_enable();
      
        // APP_ERROR_CHECK(ret);
    }
    else
    {
        // NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");

        app_usbd_enable();
        app_usbd_start();
    }
    UNUSED_RETURN_VALUE(xTaskNotifyGive(xTaskGetCurrentTaskHandle()));
    _task_started = true;
    UNUSED_RETURN_VALUE(xQueueSendToBack(task_started,&_task_started,0));
    // Enter main loop.
    for (;;)
    {
        /* Waiting for event */
        if(xQueueReceive(USB_task_api_q_handle,&api_message, 500))
        {
            switch(api_message.msg_type){
                case USB_TASK_INT:
                    while (app_usbd_event_queue_process())
                    {

                    }
                    break;
                case USB_TASK_CDC_SEND:
                    usb_send_queue();
                    break;

                case USB_TASK_CDC_RECEIVE_Q_NOT_FULL:
                    rx_done_handler();
                    break;


                default:
                    break;
            }


        }
        
    }

}

BaseType_t usb_task_aquire_cdc_receive(TickType_t wait){
    return xSemaphoreTakeRecursive(USB_CDC_rcv_mutex, wait);
}

BaseType_t usb_task_aquire_cdc_send(TickType_t wait){
    return xSemaphoreTakeRecursive(USB_CDC_send_mutex, wait);
}

BaseType_t usb_task_release_cdc_receive(){
    xSemaphoreGiveRecursive(USB_CDC_rcv_mutex);
}

BaseType_t usb_task_release_cdc_send(){
    xSemaphoreGiveRecursive(USB_CDC_rcv_mutex);
}

BaseType_t usb_task_cdc_send_char(const char c,TickType_t wait){
    BaseType_t ret;
    if(xSemaphoreTakeRecursive(USB_CDC_send_mutex,0)){
        usb_task_api_msg_t msg;
        msg.msg_type = USB_TASK_CDC_SEND;
        msg.data = NULL;

        com_data_t data;
        data.data_type = COM_CHAR;
        data.ch = c;
        if(uxQueueMessagesWaiting(USB_CDC_send_q_handle)>0){
            ret = xQueueSendToBack(USB_CDC_send_q_handle,&data, wait);
        }
        else{
            ret = xQueueSendToBack(USB_CDC_send_q_handle,&data, wait);
            ret &= xQueueSendToBack(USB_task_api_q_handle,&msg,wait);
        }
        return ret;
    }
    else{
        return pdFALSE;
    }
}

// TODO: make this work with new queue and data structures
BaseType_t usb_task_cdc_send_string(char *c, int len, SemaphoreHandle_t ownership, TickType_t wait){
    BaseType_t ret;
    if(xSemaphoreTakeRecursive(USB_CDC_send_mutex,0)){
        usb_task_api_msg_t msg;
        msg.msg_type = USB_TASK_CDC_SEND;
        msg.data = NULL;

        com_data_t data;
        data.data_type = COM_STRING;
        data.str = c;
        data.len = len;
        data.ownership = ownership;

        if(uxQueueMessagesWaiting(USB_CDC_send_q_handle)>0){
            ret = xQueueSendToBack(USB_CDC_send_q_handle,&data, wait);
        }
        else{
            ret = xQueueSendToBack(USB_CDC_send_q_handle,&data, wait);
            ret &= xQueueSendToBack(USB_task_api_q_handle,&msg,wait);
        }
        return ret;
    }
    else{
        return pdFALSE;
    }
}

BaseType_t usb_task_cdc_receive_char(char *c, bool *empty, TickType_t wait){
    BaseType_t ret;
    bool q_full = uxQueueSpacesAvailable(USB_CDC_rcv_q_handle)==0;

    if(xSemaphoreTakeRecursive(USB_CDC_rcv_mutex,0)){
        ret = xQueueReceive(USB_CDC_rcv_q_handle, c, wait);
        *empty = uxQueueMessagesWaiting(USB_CDC_rcv_q_handle)==0;

        if (q_full){
            usb_task_api_msg_t msg;
            msg.msg_type = USB_TASK_CDC_RECEIVE_Q_NOT_FULL;
            msg.data = NULL;
            xQueueSendToBack(USB_task_api_q_handle,&msg,100);
        }

        xSemaphoreGiveRecursive(USB_CDC_rcv_mutex);
        return ret;
    }
    else{
        return pdFALSE;
    }
}

bool usb_task_started(TickType_t wait){
    bool started = false;
    if(_task_started){
        return true;
    }
    else{
        while(!_task_started){
            xQueueReceive(task_started, &started,500);
        }
    }
    return true;
}

BaseType_t usb_send_queue(){
    BaseType_t ret = pdFALSE;
    com_data_t data;
    int  N;
    if (!tx_pending){
        ret = xQueueReceive(USB_CDC_send_q_handle,&data,0);
        if(ret){
            switch (data.data_type)
            {
                case COM_CHAR:
                    tx_char_buffer = data.ch;
                    tx_string_unsent=0;
                    tx_string_len = 0;
                    tx_string_ownership = NULL;
                    tx_pending=true;
                    app_usbd_cdc_acm_write(&m_app_cdc_acm, &tx_char_buffer, 1);

                    break;
                
                case COM_STRING:
                    if(data.len < NRF_DRV_USBD_EPSIZE){
                        tx_string_ptr = data.str;
                        tx_string_len = data.len;
                        tx_string_unsent = 0;
                        tx_string_ownership = data.ownership;
                        tx_pending = true;
                        app_usbd_cdc_acm_write(&m_app_cdc_acm, tx_string_ptr,tx_string_len);

                    }
                    else{
                        tx_string_ptr = data.str;
                        tx_string_len = data.len;
                        tx_string_unsent = data.len - NRF_DRV_USBD_EPSIZE;
                        tx_string_ownership = data.ownership;
                        tx_pending = true;
                        app_usbd_cdc_acm_write(&m_app_cdc_acm, tx_string_ptr, NRF_DRV_USBD_EPSIZE);
                    }
                    break;
                
                default:
                    break;
            }
        }
    }
    return ret;
}


void rx_done_handler(){
    ret_code_t ret;
            // NRF_LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));
    if(!rx_empty){
        do
        {
            /*Get amount of data transfered*/
            // size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
            // NRF_LOG_INFO("RX: size: %lu char: %c", size, m_rx_buffer[0]);

            /* Fetch data until internal buffer is empty */
            if(uxQueueSpacesAvailable(USB_CDC_rcv_q_handle)==0){
                return;
            }
            ret = xQueueSend(USB_CDC_rcv_q_handle, &m_rx_buffer[0],0);
            ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                        m_rx_buffer,
                                        READ_SIZE);

                
        } while (ret == NRF_SUCCESS);
        rx_empty = true;
    }

}
