#include "FreeRTOS.h"
#include "nrf_gpio.h"
#include "stdlib.h"
#include "string.h"
#include "task.h"
#include "queue.h"
#include "shell_task.h"

extern TaskHandle_t blinky_task_handle;
extern command_t blinky_cmd;

typedef enum{
    PERIOD,
    DUTY
} blinky_task_api_message_type_t;

typedef struct {
    blinky_task_api_message_type_t msg_type;
    int data;
} blinky_task_api_message_t;

void blinky_task(void* task_parameters);
BaseType_t set_blink_period(int period);
BaseType_t set_blink_duty(int duty);

void set_period_shell(void *data, char *str);
void set_duty_shell(void *data, char *str);
