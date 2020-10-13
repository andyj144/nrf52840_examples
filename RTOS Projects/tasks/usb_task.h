#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION false
#endif

#define READ_SIZE 1

typedef enum {
    USB_TASK_INT,
    USB_TASK_CDC_SEND,
    USB_TASK_CDC_SEND_Q_NOT_FULL,
    USB_TASK_CDC_RECEIVE_Q_NOT_FULL,
    USB_TASK_CDC_GET_INPUT,
    USB_TASK_QIN_CFG
} usb_task_api_message_type_t;


typedef enum {
    COM_STRING,
    COM_CHAR
} com_data_type_t;


typedef struct {
    usb_task_api_message_type_t msg_type;
    void *data
} usb_task_api_msg_t;

typedef struct {
    com_data_type_t data_type;
    char ch;
    char *str;
    int len;
    SemaphoreHandle_t ownership;
} com_data_t;

static void cdc_acm_user_event_handler(app_usbd_class_inst_t const * inst,
                                    app_usbd_cdc_acm_user_event_t event);

static void usbd_user_event_handler(app_usbd_event_type_t event);

static BaseType_t usb_send_queue();

static void rx_done_handler();
