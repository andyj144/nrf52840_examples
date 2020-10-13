#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

void USB_task(void * task_parameters);
extern TaskHandle_t USB_task_handle;

BaseType_t usb_task_aquire_cdc_receive(TickType_t wait);

BaseType_t usb_task_release_cdc_receive();

BaseType_t usb_task_aquire_cdc_send(TickType_t wait);

BaseType_t usb_task_release_cdc_send();

BaseType_t usb_task_cdc_send_char(const char c, TickType_t wait);

BaseType_t usb_task_cdc_send_string(char *c, int len, SemaphoreHandle_t ownership, TickType_t wait);

BaseType_t usb_task_cdc_receive_char(char *c, bool *empty, TickType_t wait);

bool usb_task_started(TickType_t wait);






