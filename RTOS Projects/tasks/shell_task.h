#include "FreeRTOS.h"
#include "semphr.h"
#include "usb_task_api.h"
#include "task.h"
#include "printf.h"


#ifndef SHELL
#define SHELL
typedef void (*command_function_t)(void*,char*);

extern TaskHandle_t shell_task_handle;


typedef struct {
    TaskHandle_t com_task;
} shell_task_parameters_t;

typedef struct command command_t;

struct command{
    const char *cmd;
    int cmd_len;
    command_function_t cmd_func;
    void *data;
    command_t *next;
};


void shell_task(void *task_parameters);
void parse_command(void *cmds, char *in);
void add_shell_command(command_t *new_cmd);
void dummy_func1(void *data, char *args);
void dummy_func2(void *data, char *args);

#endif