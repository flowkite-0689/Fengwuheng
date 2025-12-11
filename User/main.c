#include "stm32f10x.h" // Device header
#include <FreeRTOS.h>
#include <task.h>
#include "hardware_def.h"
#include "Key.h"
#include "debug.h"
#include "beep.h"
#include "oled_print.h"
#include "rtc_date.h"
#include "queue.h"
#include "unified_menu.h"
#include "index.h"
#include "esp8266.h"
#include "uart2.h"
#include "light.h"
#include "rtc_date.h"
#include "PM25.h"
// 创建队列来存储按键事件
QueueHandle_t keyQueue;     // 按键队列

static TaskHandle_t Menu_handle = NULL;
static TaskHandle_t Key_handle = NULL;
static TaskHandle_t ESP8266_handle = NULL;


/* 任务函数声明 */
static void Menu_Main_Task(void *pvParameters);
static void Key_Main_Task(void *pvParameters);
static void ESP8266_Main_Task(void *pvParameters);


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
    PM25_Init();
    
    // 初始化UART2，用于ESP8266通信
    UART2_DMA_RX_Init(115200);
    
    BEEP_Buzz(10);
   
    
    OLED_Printf_Line(0, "STM32F103C8T6");
    OLED_Printf_Line(1, "FreeRTOS V20221201");
    OLED_Printf_Line(2, "fengwuheng");
    OLED_Printf_Line(3, "v1.0.0");
    
    printf("\r\n==================================\r\n");
    printf("||     STM32F103C8T6   \t\t||\r\n");
    printf("||     FreeRTOS V20221201   \t||\r\n");
    printf("||     fengwuheng   \t\t||\r\n");
    printf("||     v1.0.0   \t\t||\r\n");
    printf("=====================================\r\n");
    Light_ADC_Init();
    printf("Light_ADC_Init OK\n");
    
    OLED_Refresh();Delay_s(1);
    OLED_Clear();
    
    printf("ESP8266初始化将在独立任务中进行...\r\n");
    OLED_Printf_Line(0, "WiFi Connecting...");
    OLED_Printf_Line(1, "Please Wait...");
    OLED_Refresh();
    
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


   
    /* 创建菜单任务 */
    xTaskCreate((TaskFunction_t)Menu_Main_Task,          /* 任务函数 */
                (const char *)"Menu_Main",               /* 任务名称 */
                (uint16_t)2024,                          /* 任务堆栈大小 */
                (void *)NULL,                           /* 任务函数参数 */
                (UBaseType_t)3,                         /* 任务优先级 */
                (TaskHandle_t *)&Menu_handle);           /* 任务控制句柄 */
    xTaskCreate(Key_Main_Task, "KeyMain", 128, NULL, 4, &Key_handle);

    printf("creat task OK\n");
    
    // 创建ESP8266任务
    xTaskCreate((TaskFunction_t)ESP8266_Main_Task,          /* 任务函数 */
                (const char *)"ESP8266_Main",               /* 任务名称 */
                (uint16_t)256,                              /* 任务堆栈大小 */
                (void *)NULL,                               /* 任务参数 */
                (UBaseType_t)2,                             /* 任务优先级 */
                (TaskHandle_t *)&ESP8266_handle);           /* 任务句柄 */
    
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

static void ESP8266_Main_Task(void *pvParameters)
{
    printf("ESP8266_Main_Task start ->\n");
    
    uint8_t first = 1;
    TickType_t heart_tick = xTaskGetTickCount();    // 
    TickType_t Publish_tick = xTaskGetTickCount();
    vTaskDelay(pdMS_TO_TICKS(2000)); // 等待ESP8266开机
    ESP8266_Receive_Start();
    
    if (ESP8266_Connect_WiFi("sgwl.az", "798798798") != 1) // 连接WiFi
    {
        printf("ESP8266 Connect WiFi Error\r\n");
        OLED_Printf_Line(0, "WiFi Error!");
        OLED_Refresh();
        vTaskDelete(NULL); // 删除自身任务
        return ;
    }
    printf("ESP8266 Connect WiFi Success\r\n");
    OLED_Printf_Line(0, "WiFi Connected!");
    OLED_Printf_Line(1, "ElevatedNetwork.lt");
    OLED_Refresh();
    
    
    if (ESP8266_Connect_Server("bemfa.com", "8344") != 1) // 连接服务器
    {
        printf("ESP8266 Connect Server Error\r\n");
        return ;
    }
    printf("ESP8266 Connect Server Success\r\n");
    if (ESP8266_TCP_Subscribe("4af24e3731744508bd519435397e4ab5", "mydht004") != 1) // 订阅主题
    {
        printf("ESP8266 TCP Subscribe Error\r\n");
        return ;
    }
    printf("ESP8266 TCP Subscribe Success\r\n");

    char time_buffer[64];
    if (ESP8266_TCP_GetTime("4af24e3731744508bd519435397e4ab5", time_buffer, sizeof(time_buffer)) != 1)
    {
        printf("ESP8266 Get Time Error\r\n");
        return ;
    }
    printf("ESP8266 Get Time Success: %s\r\n", time_buffer);
    
    // 同步到RTC
    if (RTC_SetFromNetworkTime(time_buffer) != 1)
    {
        printf("RTC Sync Failed\r\n");
    }
    else
    {
        printf("RTC Sync Success\r\n");
    }
    

   while (1)
    {
        if ((xTaskGetTickCount() - heart_tick) / 1000 >= 60)
        {
            // 发送心跳包云平台
            heart_tick = xTaskGetTickCount();
            ESP8266_TCP_Heartbeat();            
        }

        if ((xTaskGetTickCount() - Publish_tick) / 1000 >= 300 || first)
        {
            // 发布主题
            Publish_tick = xTaskGetTickCount();
            first = 0;

            if (ESP8266_TCP_Publish("4af24e3731744508bd519435397e4ab5", "mydht004", "#512#66") != 1) // 发布主题
            {
                printf("ESP8266 TCP Publish Error\r\n");
            }
            else
            {
                printf("ESP8266 TCP Publish Success\r\n");
            }
        }
        if (uart2_rx_len > 0)
        {
            uart2_rx_len = 0;
            printf("ESP8266 Receive Data: %s\r\n", uart2_buffer);   // 收到巴法云下发的数据
            
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
     
}
