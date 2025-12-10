#include "stm32f10x.h" // Device header
#include <FreeRTOS.h>
#include <task.h>
#include "hardware_def.h"
#include "Key.h"
#include "debug.h"
#include "beep.h"
#include "oled_print.h"
#include "rtc_date.h"
#include "dht11.h"
#include "queue.h"
#include "unified_menu.h"
#include "index.h"

// 创建队列来存储按键事件
QueueHandle_t keyQueue;     // 按键队列

static TaskHandle_t Menu_handle = NULL;
static TaskHandle_t Key_handle = NULL;


/* 任务函数声明 */
static void Menu_Main_Task(void *pvParameters);
static void Key_Main_Task(void *pvParameters);


int main(void)
{
    // 系统初始化开始
    TIM2_Delay_Init();
    debug_init();
    OLED_Init();

    OLED_Show_many_Tupian(tjbg,8,1);
  
    OLED_Refresh();
    Key_Init();
    Beep_Init();
    
    BEEP_Buzz(10);
   
    
    OLED_Printf_Line(0, "STM32F103C8T6");
    OLED_Printf_Line(1, "FreeRTOS V20221201");
    OLED_Printf_Line(2, "fengwuheng");
    OLED_Printf_Line(3, "v1.0.0");
    
    printf("\r\n======================================\r\n");
    printf("||     STM32F103C8T6   \t\t||\r\n");
    printf("||     FreeRTOS V20221201   \t||\r\n");
    printf("||     fengwuheng   \t\t||\r\n");
    printf("||     v1.0.0   \t\t||\r\n");
    printf("========================================\r\n");

    OLED_Refresh();Delay_s(1);
    OLED_Clear();
    
    // 初始化菜单系统
    if (menu_system_init() != 0) {
        printf("Menu system initialization failed\r\n");
        return -1;
    }
    
    // 创建并初始化首页
    menu_item_t* index_menu = index_init();
    if (index_menu == NULL) {
        printf("Index page initialization failed\r\n");
        return -1;
    }
    
    // 设置首页为根菜单
    g_menu_sys.root_menu = index_menu;
    g_menu_sys.current_menu = index_menu;

    printf("root_menu init OK\n");
    /* 创建菜单任务 */
    xTaskCreate((TaskFunction_t)Menu_Main_Task,          /* 任务函数 */
                (const char *)"Menu_Main",               /* 任务名称 */
                (uint16_t)2024,                          /* 任务堆栈大小 */
                (void *)NULL,                           /* 任务函数参数 */
                (UBaseType_t)3,                         /* 任务优先级 */
                (TaskHandle_t *)&Menu_handle);           /* 任务控制句柄 */
    xTaskCreate(Key_Main_Task, "KeyMain", 128, NULL, 4, &Key_handle);

    printf("creat task OK\n");
    
    // 添加调试信息，确认调度器启动
    printf("Starting scheduler...\n");
    
    /* 启动调度器 */
    vTaskStartScheduler();
    
    // 如果调度器启动失败，会执行到这里
    printf("ERROR: Scheduler failed to start!\n");
    // LED2_ON();
}

static void Menu_Main_Task(void *pvParameters)
{
    printf("Menu_Main_Task start ->\n");
    // 直接调用统一菜单框架的任务
    menu_task(pvParameters);
}

static void Key_Main_Task(void *pvParameters)
{
    // 直接调用统一菜单框架的按键任务
    menu_key_task(pvParameters);
}
