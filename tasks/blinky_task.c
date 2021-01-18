#include "blinky_task.h"

#ifndef LED
    #define LED NRF_GPIO_PIN_MAP(0,7)
#endif

TaskHandle_t blinky_task_handle;
QueueHandle_t blinky_api_q_handle;
command_t *blinky_cmds;

command_t blinky_cmd = {.cmd = "blinky", .cmd_len = 6, .cmd_func = parse_command, .data=NULL, .next=NULL};
command_t period_cmd = {.cmd = "period", .cmd_len = 6, .cmd_func = set_period_shell, .data = NULL, .next = NULL};
command_t duty_cmd  = {.cmd = "duty", .cmd_len = 4, .cmd_func = set_duty_shell, .data = NULL, .next = NULL};


void blinky_task(void* task_parameters){
    int period;
    int on;
    int off;
    BaseType_t ret;
    bool led_on=true;


    blinky_api_q_handle = xQueueCreate(1, sizeof(blinky_task_api_message_t));
    blinky_task_api_message_t msg;
    blinky_cmds = &period_cmd;
    period_cmd.next = &duty_cmd;

    blinky_cmd.data = (void*)blinky_cmds;

    period = 100;
    on = 50;
    off = 50;
    while(true){  
        if(led_on){
            vTaskDelay(on);
        }
        else{
            vTaskDelay(off);
        } 
        nrf_gpio_pin_toggle(LED);
        led_on ^= 1;
        ret = xQueueReceive(blinky_api_q_handle, &msg, 0);
        if(ret){
            switch (msg.msg_type)
            {
            case PERIOD:
                period = msg.data;
                break;
            case DUTY:
                on = (int)((float)(msg.data)/100.0 * period);
                off = period - on;
            default:
                break;
            }
        }
        // taskYIELD();
    }
}

BaseType_t set_blinky_period(int period){
    BaseType_t ret;
    blinky_task_api_message_t msg ={.msg_type = PERIOD, .data = NULL};
    msg.data = period;
    ret = xQueueSendToBack(blinky_api_q_handle, &msg, 100);
    return ret;
}

void set_period_shell(void *data, char *str){
    char *del;
    int param_len;
    char buffer[10] = {NULL};
    
    del = strpbrk(str," \r");
    param_len = del-str;
    if (param_len>9) param_len=9;
    memcpy(buffer,str,param_len);
    set_blinky_period(atoi(buffer));
}

BaseType_t set_blinky_duty(int duty){
    BaseType_t ret;
    blinky_task_api_message_t msg ={.msg_type = DUTY, .data = NULL};
    msg.data = duty;
    ret = xQueueSendToBack(blinky_api_q_handle, &msg, 100);
    return ret;
}

void set_duty_shell(void *data, char *str){
    char *del;
    int param_len;
    char buffer[10] = {NULL};
    
    del = strpbrk(str," \r");
    param_len = del-str;
    if (param_len>9) param_len=9;
    memcpy(buffer,str,param_len);
    set_blinky_duty(atoi(buffer));
}