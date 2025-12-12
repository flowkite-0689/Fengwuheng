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
#include "sensordata.h"
// 创建队列来存储按键事件
QueueHandle_t keyQueue; // 按键队列

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

    // OLED_Show_many_Tupian(tjbg, 8, 1);

    OLED_Refresh();
    Key_Init();
    Beep_Init();
    PM25_Init();

    // 初始化UART2，用于ESP8266通信
    UART2_DMA_RX_Init(115200);

    BEEP_Buzz(10);

    // OLED_Printf_Line(0, "STM32F103C8T6");
    // OLED_Printf_Line(1, "FreeRTOS V20221201");
    // OLED_Printf_Line(2, "fengwuheng");
    // OLED_Printf_Line(3, "v1.0.0");

    printf("\r\n==================================\r\n");
    printf("||     STM32F103C8T6   \t\t||\r\n");
    printf("||     FreeRTOS V20221201   \t||\r\n");
    printf("||     fengwuheng   \t\t||\r\n");
    printf("||     v1.0.0   \t\t||\r\n");
    printf("=====================================\r\n");
    Light_ADC_Init();
    printf("Light_ADC_Init OK\n");

    OLED_Refresh();
    // Delay_s(1);
    OLED_Clear();

    // printf("ESP8266初始化将在独立任务中进行...\r\n");
    // OLED_Printf_Line(0, "WiFi Connecting...");
    // OLED_Printf_Line(1, "Please Wait...");
    OLED_Refresh();

    // 初始化传感器数据模块
    SensorData_Init();
    printf("Sensor data initialization complete\r\n");

    // 初始化菜单系统
    if (menu_system_init() != 0)
    {
        printf("Menu system initialization failed\r\n");
        return -1;
    }

    // 创建并初始化首页
    menu_item_t *index_menu = index_init();
    if (index_menu == NULL)
    {
        printf("Index page initialization failed\r\n");
        return -1;
    }

    // 设置首页为根菜单
    g_menu_sys.root_menu = index_menu;
    g_menu_sys.current_menu = index_menu;

    /* 创建菜单任务 */
    xTaskCreate((TaskFunction_t)Menu_Main_Task, /* 任务函数 */
                (const char *)"Menu_Main",      /* 任务名称 */
                (uint16_t)2024,                 /* 任务堆栈大小 */
                (void *)NULL,                   /* 任务函数参数 */
                (UBaseType_t)4,                 /* 任务优先级 */
                (TaskHandle_t *)&Menu_handle);  /* 任务控制句柄 */
    xTaskCreate(Key_Main_Task, "KeyMain", 128, NULL, 4, &Key_handle);

    printf("creat task OK\n");

    // 创建传感器数据采集任务
    SensorData_CreateTask();
    printf("SensorData task created\n");

    // 创建ESP8266任务
    xTaskCreate((TaskFunction_t)ESP8266_Main_Task, /* 任务函数 */
                (const char *)"ESP8266_Main",      /* 任务名称 */
                (uint16_t)300,                     /* 任务堆栈大小 */
                (void *)NULL,                      /* 任务参数 */
                (UBaseType_t)2,                    /* 任务优先级 */
                (TaskHandle_t *)&ESP8266_handle);  /* 任务句柄 */

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
    TickType_t heart_tick = xTaskGetTickCount(); //
    TickType_t Publish_tick = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(2000)); // 等待ESP8266开机
    ESP8266_Receive_Start();
    
    uint8_t retry_count = 0;
    const uint8_t max_retries = 5;
    
    while (!wifi_connected && retry_count < max_retries)
    {
        retry_count++;
        printf("WiFi connection attempt %d/%d\r\n", retry_count, max_retries);
        // OLED_Printf_Line(0, "WiFi Connecting");
        // OLED_Printf_Line(1, "Attempt %d/%d", retry_count, max_retries);
        // OLED_Refresh();
        
        if (ESP8266_Connect_WiFi("ElevatedNetwork.lt", "798798798") == 1) // 连接WiFi
        {
            printf("ESP8266 Connect WiFi Success\r\n");
            wifi_connected = 1;
        }
        else
        {
            printf("ESP8266 Connect WiFi Error, attempt %d/%d\r\n", retry_count, max_retries);
            if (retry_count < max_retries)
            {
                // 等待5秒后重试
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }
    }
    
    if (!wifi_connected)
    {
        printf("WiFi connection failed after %d attempts, entering retry loop\r\n", max_retries);
        
        
        // 进入无限重试循环
        while (!wifi_connected)
        {
            vTaskDelay(pdMS_TO_TICKS(30000)); // 每30秒重试一次
            printf("Retrying WiFi connection...\r\n");
            if (ESP8266_Connect_WiFi("ElevatedNetwork.lt", "798798798") == 1)
            {
                printf("ESP8266 Connect WiFi Success after retry\r\n");
                wifi_connected = 1;
                OLED_Printf_Line(0, "WiFi Connected!");
                OLED_Refresh();
            }
        }
    }

    retry_count = 0;
    while (!Server_connected && retry_count < max_retries)
    {
        retry_count++;
        printf("Server connection attempt %d/%d\r\n", retry_count, max_retries);
        if (ESP8266_Connect_Server("bemfa.com", "8344") == 1) // 尝试连接服务器
        {
            printf("ESP8266 Connect Server Success\r\n");
            Server_connected = 1;
        }
        else
        {
            printf("ESP8266 Connect Server Error, attempt %d/%d\r\n", retry_count, max_retries);
            if (retry_count < max_retries)
            {
                // 等待5秒后重试
                vTaskDelay(pdMS_TO_TICKS(5000));

            }
        }
    }

    if (!Server_connected)
    {
        printf("Server connection failed after %d attempts, entering retry loop\r\n", max_retries);
       
        while (!Server_connected)
        {
            vTaskDelay(pdMS_TO_TICKS(30000)); // 每30秒重试一次
            printf("Retrying Server connection...\r\n");
            if (ESP8266_Connect_Server("bemfa.com", "8344") == 1)
            {
                printf("ESP8266 Connect Server Success after retry\r\n");
                Server_connected = 1;
            }
        }
    }
    
    printf("ESP8266 Connect Server Success\r\n");
    if (ESP8266_TCP_Subscribe("4af24e3731744508bd519435397e4ab5", "mydht004") != 1) // 订阅主题
    {
        printf("ESP8266 TCP Subscribe  mydht004 Error\r\n");
        return;
    }
    printf("ESP8266 TCP Subscribe  mydht004 Success\r\n");

    //
    if (ESP8266_TCP_Subscribe("4af24e3731744508bd519435397e4ab5", "myMP25004") != 1) // 订阅主题
    {
        printf("ESP8266 TCP Subscribe  myMP25004 Error\r\n");
        return;
    }
    printf("ESP8266 TCP Subscribe  myMP25004 Success\r\n");

    //

    if (ESP8266_TCP_Subscribe("4af24e3731744508bd519435397e4ab5", "myLUX004") != 1) // 订阅主题
    {
        printf("ESP8266 TCP Subscribe  myLuxGet Error\r\n");
        return;
    }
    printf("ESP8266 TCP Subscribe  myLuxGet Success\r\n");

    char time_buffer[64];
    if (ESP8266_TCP_GetTime("4af24e3731744508bd519435397e4ab5", time_buffer, sizeof(time_buffer)) != 1)
    {
        printf("ESP8266 Get Time Error\r\n");
        return;
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

        if ((xTaskGetTickCount() - Publish_tick) / 1000 >= 15 || first)
        {
            // 发布主题
            Publish_tick = xTaskGetTickCount();
            first = 0;

            //

            char data[16];
            // 发布主题 :mydht004
            snprintf(data, sizeof(data), "#%d.%d#%d",
                     SensorData.dht11_data.temp_int,
                     SensorData.dht11_data.temp_deci,
                     SensorData.dht11_data.humi_int);

            if (ESP8266_TCP_Publish("4af24e3731744508bd519435397e4ab5", "mydht004", data) != 1) // 发布主题
            {
                printf("ESP8266 TCP Publish mydht004 Error\r\n");
            }
            else
            {
                printf("ESP8266 TCP Publish mydht004 Success\r\n");
            }
            // 发布主题 :myLuxGet
            snprintf(data, sizeof(data), "#%d",
                     SensorData.light_data.lux);

            if (ESP8266_TCP_Publish("4af24e3731744508bd519435397e4ab5", "myLUX004", data) != 1) // 发布主题
            {
                printf("ESP8266 TCP Publish myLuxGet Error\r\n");
            }
            else
            {
                printf("ESP8266 TCP Publish myLuxGet Success\r\n");
            }

            // 发布主题 : myMP25004
            snprintf(data, sizeof(data), "#%0.1f#%d",
                     SensorData.pm25_data.pm25_value,
                     SensorData.pm25_data.level);
            if (ESP8266_TCP_Publish("4af24e3731744508bd519435397e4ab5", "myMP25004", data) != 1) // 发布主题
            {
                printf("ESP8266 TCP Publish myMP25004 Error\r\n");
            }
            else
            {
                printf("ESP8266 TCP Publish myMP25004 Success\r\n");
            }
        }
        if (uart2_rx_len > 0)
        {
            uart2_rx_len = 0;
            printf("ESP8266 Receive Data: %s\r\n", uart2_buffer); // 收到巴法云下发的数据
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
