#include "system.h"

#define UTIL_TRACE_STACK_MAX_TASK 32

TaskHandle_t g_util_trace_stack_task_handle[ UTIL_TRACE_STACK_MAX_TASK ];

uint8_t g_util_trace_stack_task_index;

void util_trace_stack_add_task( TaskHandle_t handle )
{
    if(g_util_trace_stack_task_index == UTIL_TRACE_STACK_MAX_TASK)
    {
        system_printf("Trace Stack Task Full. Need to increase UTIL_TRACE_STACK_MAX_TASK\n");
        return;
    }
    g_util_trace_stack_task_handle [g_util_trace_stack_task_index++ ] = handle;
    system_printf("No %d. Task Name(%s) Handle(%d) Added to Stack Trace\n" , g_util_trace_stack_task_index , pcTaskGetName(handle) , handle );
}
static void iterate_task(void (*callback)(TaskStatus_t *, uint32_t *, void *), uint32_t *jiffies, void *arg)
{
    TaskStatus_t *info;
    int i, nr_task;


    nr_task = uxTaskGetNumberOfTasks();
    info = os_malloc(nr_task * sizeof(*info));
    if (info == NULL)
        return;


    nr_task = uxTaskGetSystemState(info, nr_task, jiffies);
    for (i = 0; i < nr_task; i++) {
        callback(&info[i], jiffies, arg);
    }


    os_free(info);
    return;
}
#define tick_to_second(x) ((x) / (configTICK_RATE_HZ))


struct xtime {
    int dd;
    int hh;
    int mm;
    int ss;
};


static void ddhhmmss(long seconds, struct xtime *time) {


    if (!time)
        return;


    time->dd = seconds / (24 * 60 * 60);
    seconds -= time->dd * (24 * 60 * 60);
    time->hh = seconds / (60 * 60);
    seconds -= time->hh * (60 * 60);
    time->mm = seconds / 60;
    seconds -= time->mm * 60;
    time->ss = seconds;
};


static void print_task_info(TaskStatus_t *ti, uint32_t *jiffies, void *data)
{
    char state[] = {'X', 'R', 'B', 'S', 'D', 'I'};
    struct xtime time;
    unsigned long ratio;


    ratio = ((uint64_t)ti->ulRunTimeCounter) * 1000 / (*jiffies + 1);
    ddhhmmss(tick_to_second(ti->ulRunTimeCounter), &time);


    system_printf("%4lu%5lu%6d%3c%5lu.%1d%5d:%02d:%02d   %-32s\n",
           ti->xTaskNumber,
           ti->uxCurrentPriority,
           ti->usStackHighWaterMark,
           state[ti->eCurrentState],
           ratio / 10, (int) ratio % 10,
           time.hh, time.mm, time.ss,
           ti->pcTaskName);
}


static void show_task_stack_info()
{
    uint32_t jiffies;


    system_printf("\n");
    system_printf(" \n");
    system_printf("%4s%5s%6s%3s%7s%11s   %-44s\n",
       "PID", "PR", "STWM", "S", "%CPU+", "TIME+", "TASK");


    iterate_task(print_task_info, &jiffies, NULL);
}

int cmd_show_stack(cmd_tbl_t *t, int argc, char *argv[])
{
    uint8_t i;
    uint32_t size;
#ifndef UTIL_TRACE_STACK
    system_printf("You should define UTIL_TRACE_STACK in Makefile\n");
    return CMD_RET_SUCCESS;
#endif
    show_task_stack_info();

    for(i = 0 ; i < g_util_trace_stack_task_index ; i ++)
    {
        size = uxTaskGetStackHighWaterMark( g_util_trace_stack_task_handle[i] ) * 4;
        system_printf("No %2d. Minimum Free Stack Size = %10d , Task Name(%s)  \n" , i , size , pcTaskGetName( g_util_trace_stack_task_handle[i] ) );
    }
    system_printf("Current Free Heap Size = %10d\n", xPortGetFreeHeapSize() );
    system_printf("Minumum Free Heap Size = %10d\n", xPortGetMinimumEverFreeHeapSize() );
	return CMD_RET_SUCCESS;
}
SUBCMD(show,
       stack,
       cmd_show_stack,
       "displays stack status",
       "show stack");
