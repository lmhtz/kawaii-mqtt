/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-27 23:10:36
 * @LastEditTime: 2020-02-24 01:18:46
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
/** synchronous asynchronous log output framework */

#include "salof.h"

#ifdef KAWAII_MQTT_USE_LOG

#ifndef KAWAII_MQTT_SALOF_BUFF_SIZE
    #define     KAWAII_MQTT_SALOF_BUFF_SIZE     (1024U)
#endif 
#ifndef KAWAII_MQTT_SALOF_FIFO_SIZE
    #define     KAWAII_MQTT_SALOF_FIFO_SIZE     (2048U)
#endif 

static int salof_out(char *buf, int len);
void salof_handler( void );

#ifdef KAWAII_MQTT_USE_SALOF
#include <string.h>
static fifo_t _salof_fifo = NULL;
static int _len;
static char _out_buff[KAWAII_MQTT_SALOF_BUFF_SIZE];

#ifndef KAWAII_MQTT_USE_IDLE_HOOK
static salof_tcb _salof_task;
void salof_task(void *parm);
#endif

#endif

static char _format_buff[KAWAII_MQTT_SALOF_BUFF_SIZE];

int salof_init(void)
{
#ifdef KAWAII_MQTT_USE_SALOF
    _salof_fifo = fifo_create(KAWAII_MQTT_SALOF_FIFO_SIZE);
    if(_salof_fifo == NULL)
        return -1;

#ifndef KAWAII_MQTT_USE_IDLE_HOOK
    _salof_task = salof_task_create("salof_task", salof_task, NULL, 
                                    KAWAII_MQTT_SALOF_THREAD_STACK_SIZE, 
                                    KAWAII_MQTT_SALOF_TASK_PRIO, 
                                    KAWAII_MQTT_SALOF_THREAD_TICK);
    if(_salof_task == NULL)
        return -1;
#else
    rt_thread_idle_sethook(salof_handler);
#endif
#endif
    return 0;
}


void salof(const char *fmt, ...)
{
    va_list args;
    int len;
    va_start(args, fmt);

    len = format_nstr(_format_buff, KAWAII_MQTT_SALOF_BUFF_SIZE - 1, fmt, args);

    if(len > KAWAII_MQTT_SALOF_BUFF_SIZE)
        len = KAWAII_MQTT_SALOF_BUFF_SIZE - 1;

#ifdef KAWAII_MQTT_USE_SALOF
    fifo_write(_salof_fifo, _format_buff, len, 100);
#else
    salof_out(_format_buff, len);
#endif

  va_end(args);
}

static int salof_out(char *buf, int len)
{
    return send_buff(buf, len);
}

#ifdef KAWAII_MQTT_USE_SALOF
void salof_handler( void )
{
    _len = fifo_read(_salof_fifo, _out_buff, sizeof(_out_buff), 0);
    if(_len > 0) {
        salof_out((char *)_out_buff, _len);
        memset(_out_buff, 0, _len);
    }
}


#ifndef KAWAII_MQTT_USE_IDLE_HOOK
void salof_task(void *parm)
{   
    (void)parm;
    while(1) {
        salof_handler();
    } 
}
#endif

#endif

#endif
