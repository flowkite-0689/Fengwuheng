#include "rtc_date.h"


// ================== 全局变量 ==================
uint16_t MyRTC_Time[7] = {2005, 2, 13, 7, 30, 0}; // 
myRTC_data RTC_data = {0};

// 星期字符串（对齐 OLED 显示）
static const char *weekday_str[] = {
    "   Sunday",    // 0
    "   Monday",    // 1
    "  Tuesday",    // 2
    "Wednesday",    // 3
    " Thursday",    // 4
    "   Friday",    // 5
    " Saturday"     // 6
};

// ================== 工具函数：儒略日计算 ==================
// 将年月日转换为儒略日（整数），支持 1900~2100
// static uint32_t DateToJulianDay(uint16_t year, uint8_t month, uint8_t day) {
//     if (month <= 2) {
//         year--;
//         month += 12;
//     }
//     uint32_t A = year / 100;
//     uint32_t B = 2 - A + (A / 4);
//     return (uint32_t)(365.25 * (year + 4716)) + (uint32_t)(30.6001 * (month + 1)) + day + B - 1524;
// }

// // 将儒略日转换为年月日（输出参数）
// static void JulianDayToDate(uint32_t jd, uint16_t *year, uint8_t *month, uint8_t *day) {
//     uint32_t a = jd + 32044;
//     uint32_t b = (4 * a + 3) / 146097;
//     uint32_t c = a - (146097 * b) / 4;
//     uint32_t d = (4 * c + 3) / 1461;
//     uint32_t e = c - (1461 * d) / 4;
//     uint32_t m = (5 * e + 2) / 153;

//     *day   = (uint8_t)(e - (153 * m + 2) / 5 + 1);
//     *month = (uint8_t)(m + 3 - 12 * (m / 10));
//     *year  = (uint16_t)(100 * b + d - 4800 + (m / 10));
// }

// 使用 Zeller 公式
static uint8_t ZellerWeekDay(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    uint32_t K = year % 100;
    uint32_t J = year / 100;
    uint32_t h = (day + (13 * (month + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    // h: 0=Sat, 1=Sun, 2=Mon, ..., 6=Fri
    // 转为 0=Sun, 1=Mon, ..., 6=Sat
    uint8_t wd = (h + 6) % 7; // Sat(0)→6, Sun(1)→0, Mon(2)→1, ...
    return wd;
}

// ================== 时间 <-> 秒数转换 ==================
// 基准时间：2000-01-01 00:00:00 UTC → 秒数 = 0
// 支持范围：2000-01-01 至 2100-12-31（足够用）

#define RTC_EPOCH_YEAR 2000

// 将 (年,月,日,时,分,秒) 转为自 2000-01-01 00:00:00 UTC 起的秒数
static uint32_t DateTimeToSeconds(uint16_t year, uint8_t month, uint8_t day,
                                  uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (year < RTC_EPOCH_YEAR) year = RTC_EPOCH_YEAR;
    if (year > 2099) year = 2099;

    // 计算从 2000-01-01 到 year-01-01 的总天数
    uint32_t days = 0;
    for (uint16_t y = RTC_EPOCH_YEAR; y < year; y++) {
        days += 365 + ((y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 1 : 0);
    }

    // 加上当年已过天数
    static const uint8_t days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    uint8_t dim = days_in_month[month - 1];
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        dim = 29;
    }
    if (day > dim) day = dim;

    for (uint8_t m = 1; m < month; m++) {
        uint8_t d = days_in_month[m - 1];
        if (m == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
            d = 29;
        }
        days += d;
    }
    days += day - 1; // 1号算第0天

    // 总秒数
    uint32_t total_sec = days * 86400UL
                       + hours * 3600UL
                       + minutes * 60UL
                       + seconds;
    return total_sec;
}

// 将秒数（自 2000-01-01 00:00:00 UTC）转为 DateTime
static void SecondsToDateTime(uint32_t seconds,
                              uint16_t *year, uint8_t *month, uint8_t *day,
                              uint8_t *hours, uint8_t *minutes, uint8_t *seconds_out) {
    uint32_t days = seconds / 86400;
    uint32_t secs_remain = seconds % 86400;

    *hours   = (uint8_t)(secs_remain / 3600);
    *minutes = (uint8_t)((secs_remain % 3600) / 60);
    *seconds_out = (uint8_t)(secs_remain % 60);

    // 从 2000 开始逐年减
    *year = RTC_EPOCH_YEAR;
    while (1) {
        uint32_t year_days = 365 + (((*year) % 4 == 0 && ((*year) % 100 != 0 || (*year) % 400 == 0)) ? 1 : 0);
        if (days < year_days) break;
        days -= year_days;
        (*year)++;
    }

    // 逐月减
    static const uint8_t days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    *month = 1;
    while (*month <= 12) {
        uint8_t d = days_in_month[*month - 1];
        if (*month == 2 && ((*year) % 4 == 0 && ((*year) % 100 != 0 || (*year) % 400 == 0))) {
            d = 29;
        }
        if (days < d) break;
        days -= d;
        (*month)++;
    }
    *day = (uint8_t)(days + 1);
}

// ================== RTC 底层操作 ==================

#define LSE_TIMEOUT_S   5  // 5秒超时
#define SYSCLK_FREQ_HZ  72000000
uint8_t RTC_WaitForSynchro_Debug(void)
{
    uint32_t timeout = 0;
    printf("[RTC] Clear RSF flag...\n");
    RTC->CRL &= (uint16_t)~RTC_FLAG_RSF; // 清除 RSF

    while ((RTC->CRL & RTC_FLAG_RSF) == (uint16_t)RESET)
    {
        Delay_ms(1);
        if (++timeout > 500) {
            printf("[RTC ERROR] RSF timeout! CRL=0x%04X\n", RTC->CRL);
            printf("Check: 1. PC14/PC15 IN_FLOATING? 2. VBAT powered? 3. LSE crystal?\n");
            return 0;
        }
        
    }
    printf("\n[RTC OK] RSF set! CRL=0x%04X\n", RTC->CRL);
    return 1;
}
// 初始化 RTC（LSE 为主，失败回退 LSI）
void MyRTC_Init(void)
{

 
    printf("MyRTC init...\n");
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A6) {
        printf("First RTC config. Trying LSE...\n");

//  用·LSI
        // goto USE_LSI;
        // ==== 尝试 LSE ====
        BKP_DeInit();
        RCC_LSEConfig(RCC_LSE_ON);
        uint32_t timeout = 0;
        printf("RCC_LSEConfig(RCC_LSE_ON);");
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET) {
            Delay_s(1);
            printf("%d",timeout);
            OLED_Printf_Line(0,"LSE time out : %d",(5-timeout));
            OLED_DrawProgressBar(0,32,128,10,timeout,0,LSE_TIMEOUT_S,1,1,0);
            OLED_Refresh_Dirty();
            if (++timeout > LSE_TIMEOUT_S) {
                printf("LSE timeout! Falling back to LSI.\n");
                OLED_Clear();
                goto USE_LSI;
            }
        }
        printf("flag 1");
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);
        
        printf(" RTC_WaitForSynchro");
        OLED_Printf_Line(1,"wait for Synchro...");
        OLED_Refresh_Dirty();
        if (!RTC_WaitForSynchro_Debug())
        {
            goto USE_LSI;
        }
        
        printf(" RTC_WaitForLastTask");
        RTC_WaitForLastTask();

        printf("flag 2");
        RTC_SetPrescaler(32767); // 32768 - 1 → 1Hz
        RTC_WaitForLastTask();

        MyRTC_SetTime(); // 写入初始时间
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A6);
        printf("RTC init with LSE OK!\n");
        return;

    USE_LSI:
        // ==== 回退 LSI ====
        RCC_LSICmd(ENABLE);
        timeout = 0;
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET) {
            Delay_ms(1);
            if (++timeout > 100) break; // LSI 通常 <10ms
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();

        // LSI 频率约 40kHz，需调整分频
        // 精确值需校准，此处按 38000Hz 估算：38000 - 1 = 37999
        // 实际可用 RTC_Adjust() 校准，此处简化：
        RTC_SetPrescaler(37999); // ≈1Hz
        RTC_WaitForLastTask();

        MyRTC_SetTime();
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A6); // 标志 LSI 模式
        printf("RTC init with LSI OK!\n");

    } else {
        // 不是首次：恢复时钟源（从备份寄存器读）
        if (BKP_ReadBackupRegister(BKP_DR1) == 0xA5A6) {
            printf("Resuming with LSI...\n");
            RCC_LSICmd(ENABLE);
            uint32_t t = 0;
            while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && t++ < 100) Delay_ms(1);
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        } else {
            printf("Resuming with LSE...\n");
            RCC_LSEConfig(RCC_LSE_ON);
            uint32_t t = 0;
            while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && t++ < LSE_TIMEOUT_S) Delay_s(1);
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        }
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
    }

    printf("RTC init OK!\n");
}

// 写全局时间数组到 RTC（UTC 时间）
void MyRTC_SetTime(void)
{
    // MyRTC_Time 中是 **本地时间（东八区）**
    // → 转为 UTC 再存入 RTC
    uint16_t year  = MyRTC_Time[0];
    uint8_t  month = MyRTC_Time[1];
    uint8_t  day   = MyRTC_Time[2];
    uint8_t  hour  = MyRTC_Time[3];
    uint8_t  min   = MyRTC_Time[4];
    uint8_t  sec   = MyRTC_Time[5];

    // 减 8 小时转 UTC（注意跨天）
    int32_t total_sec = (int32_t)DateTimeToSeconds(year, month, day, hour, min, sec);
    total_sec -= 8 * 3600; // UTC = CST - 8h

    if (total_sec < 0) {
        // 极端情况：2000-01-01 00:00:00 之前，设为 2000-01-01 00:00:00 UTC
        total_sec = 0;
    }

    RTC_SetCounter((uint32_t)total_sec);
    RTC_WaitForLastTask();
}

// 从 RTC 读取（UTC 秒数）→ 更新 MyRTC_Time 和 RTC_data（转为本地时间）
void MyRTC_ReadTime(void)
{
    uint32_t rtc_sec_utc = RTC_GetCounter();

    // 转为本地时间（UTC + 8h）
    uint32_t local_sec = rtc_sec_utc + 8 * 3600;

    uint16_t year; uint8_t mon, day, hour, min, sec;
    SecondsToDateTime(local_sec, &year, &mon, &day, &hour, &min, &sec);

    // 更新全局数组（本地时间）
    MyRTC_Time[0] = year;
    MyRTC_Time[1] = mon;
    MyRTC_Time[2] = day;
    MyRTC_Time[3] = hour;
    MyRTC_Time[4] = min;
    MyRTC_Time[5] = sec;

    // 计算星期
    uint8_t wday = ZellerWeekDay(year, mon, day); // 0=Sun ... 6=Sat
    MyRTC_Time[6] = (wday == 0) ? 7 : wday;       // 1=Mon ... 7=Sun

    // 更新结构体
    RTC_data.year = year;
    RTC_data.mon = mon;
    RTC_data.day = day;
    RTC_data.hours = hour;
    RTC_data.minutes = min;
    RTC_data.seconds = sec;
    RTC_data.weekday = (wday <= 6) ? weekday_str[wday] : "Unknown";
}

// 手动设置（输入为 **本地时间**）
void RTC_SetDateTime_Manual(uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    // 参数校验
    if (year < 2000) year = 2000;
    if (year > 2099) year = 2099;
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    if (seconds > 59) seconds = 59;

    MyRTC_Time[0] = year;
    MyRTC_Time[1] = month;
    MyRTC_Time[2] = day;
    MyRTC_Time[3] = hours;
    MyRTC_Time[4] = minutes;
    MyRTC_Time[5] = seconds;

    MyRTC_SetTime(); // 自动转 UTC 存入 RTC

    printf("RTC set to %04d-%02d-%02d %02d:%02d:%02d (Local)\n",
           year, month, day, hours, minutes, seconds);
}
void RTC_SetTime_Manual(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    if (seconds > 59) seconds = 59;

    MyRTC_Time[3] = hours;
    MyRTC_Time[4] = minutes;
    MyRTC_Time[5] = seconds;

    MyRTC_SetTime(); 
}
void RTC_SetDate_Manual(uint16_t year, uint8_t month, uint8_t day)
{
    // 参数校验
    if (year < 2000U) year = 2000U;
    if (year > 2099U) year = 2099U;
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;

    
    MyRTC_Time[0] = year;
    MyRTC_Time[1] = month;
    MyRTC_Time[2] = day;

    MyRTC_SetTime(); // 自动转 UTC 存入 RTC
}

// 从网络时间字符串设置RTC（格式：2021-06-11 16:39:27）
uint8_t RTC_SetFromNetworkTime(const char *time_str)
{
    if (time_str == NULL) {
        printf("RTC_SetFromNetworkTime: NULL time string\n");
        return 0;
    }
    
    printf("Parsing network time: %s\n", time_str);
    
    // 解析时间字符串格式：2021-06-11 16:39:27
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    
    if (sscanf(time_str, "%hu-%hhu-%hhu %hhu:%hhu:%hhu", 
               &year, &month, &day, &hour, &minute, &second) != 6) {
        printf("RTC_SetFromNetworkTime: Failed to parse time string\n");
        return 0;
    }
    
    // 验证解析结果
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || 
        day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
        printf("RTC_SetFromNetworkTime: Invalid time values\n");
        return 0;
    }
    
    printf("Parsed time: %04d-%02d-%02d %02d:%02d:%02d\n", 
           year, month, day, hour, minute, second);
    
    // 使用现有的手动设置函数
    RTC_SetDateTime_Manual(year, month, day, hour, minute, second);
    
    // 读取确认设置结果
    MyRTC_ReadTime();
    printf("RTC set to: %04d-%02d-%02d %02d:%02d:%02d\n", 
           MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2],
           MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
    
    return 1;
}
