#include "shell_task.h"
#include "stdio.h"

#define MAX_CMD_LEN 100

TaskHandle_t shell_task_handle;
command_t *cmds;

char input_buffer[MAX_CMD_LEN];
int len = 0;

char  msg[MAX_CMD_LEN];
int msg_len;
static SemaphoreHandle_t msg_ownership;

command_t dummy1 = {.cmd = "dummy1",.cmd_len = 6, .cmd_func = dummy_func1, .data=NULL, .next=NULL};
command_t dummy2 = {.cmd = "dummy2",.cmd_len = 6, .cmd_func = dummy_func2, .data=NULL, .next=NULL};


void shell_task(void *task_parameters){
    
    BaseType_t ret = pdFALSE;
    bool empty = true;
    msg_ownership = xSemaphoreCreateBinary();
    add_shell_command(&dummy1);
    add_shell_command(&dummy2);
    

    // msg = pvPortMalloc(50 * sizeof(char));
    if(usb_task_started(500)){

    }

    while (!ret){
            ret = usb_task_aquire_cdc_receive(100);
            ret &= usb_task_aquire_cdc_send(100);
    }
    while(true){
        do
        {
            if(usb_task_cdc_receive_char(&input_buffer[len],&empty,100)){

                usb_task_cdc_send_char(input_buffer[len],100);
                 ++len;
                 if(input_buffer[len-1] == '\r'){
                    input_buffer[len] = NULL;
                    usb_task_cdc_send_char('\n',100);
                    parse_command(cmds, input_buffer);
                 }
            
                if(len==MAX_CMD_LEN-1){
                    xSemaphoreTake(msg_ownership,1000);
                    msg_len = snprintf(msg,MAX_CMD_LEN,"\n\rMax command length exceeded = %d, exceeded\n\r", MAX_CMD_LEN);
                    usb_task_cdc_send_string(msg,msg_len,msg_ownership,100);
                    len = 0;
                }
            }
        }while(!empty);
    }
}

void aquire_input(){

}

void aquire_output(){

}

void release_input(){

}

void release_output(){

}

void parse_command(void *cmds, char *in){
    char *del;
    int token_len;
    command_t *cmd = (command_t*)cmds;
    
    del = strpbrk(in," \r");
    msg_len = del-in;
    if(!cmds){
        return;
    }

    do{
        if (memcmp(in,cmd->cmd, cmd->cmd_len)==0){
            (cmd -> cmd_func)(cmd->data,del+1);
            break;
        }
    }while(cmd = cmd->next);
    // xSemaphoreTake(msg_ownership,1000);
    // for(int i=0; i<(del-in);i++){
    //     msg[i]=in[i];
    // }
    // usb_task_cdc_send_string(msg,msg_len,msg_ownership,100);


    len = 0;
}


void add_shell_command(command_t *new_cmd){
    if(!cmds){
        cmds = new_cmd;
    }
    else{
        new_cmd->next = cmds;
        cmds = new_cmd;
    }
}


void dummy_func1(void *data, char *args){
    char *del;
    int token_len;
    int i;
    
    del = strpbrk(args," \r");
    msg_len = del-args;
    
    xSemaphoreTake(msg_ownership,1000);
    for(i=0; i<(del-args);i++){
        msg[i]=args[i];
    }
    msg[i]='\r';
    msg[i+1]='\n';
    usb_task_cdc_send_string(msg,msg_len+2,msg_ownership,100);
}


void dummy_func2(void *data, char *args){
    char *del;
    int token_len;
    
    del = strpbrk(args," \r");
    msg_len = del-args;
    
    xSemaphoreTake(msg_ownership,1000);
    msg_len = snprintf(msg,MAX_CMD_LEN,"The parameter length is: %d\n\r",msg_len);
    usb_task_cdc_send_string(msg,msg_len,msg_ownership,100);
}

