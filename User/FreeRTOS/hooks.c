#include <FreeRTOS.h>
#include <task.h>
#include "debug.h"


void vApplicationIdleHook( void )
{
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    printf("!!! Stack Overflow in task: %s !!!\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
void vApplicationTickHook( void )
{
   
}
void vApplicationMallocFailedHook( void )
{
    printf("!!! Memory Allocation Failed !!!\n");
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
