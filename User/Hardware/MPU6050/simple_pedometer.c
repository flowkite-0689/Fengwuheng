#include "simple_pedometer.h"
#include "math.h"
#include <stdlib.h>
#include "debug.h"
// 包含步数存储函数

// 全局步数变量
unsigned long g_step_count = 0;

// 计步器状态结构体
typedef struct {
    short last_acceleration;        // 上一次的合加速度
    short peak_threshold;           // 峰值阈值
    short noise_threshold;          // 噪声阈值
    short min_step_interval;        // 最小步间隔（毫秒）
    unsigned long last_step_time;   // 上次步数检测时间
    short step_state;               // 步数检测状态（0-等待波峰，1-等待波谷）
} SimplePedometer;

// 全局计步器实例
static SimplePedometer pedometer;

/**
 * @brief 初始化简单计步器
 */
void simple_pedometer_init(void)
{
    // 初始化参数
    g_step_count = 0;
    pedometer.last_acceleration = 0;
    // 降低阈值以提高灵敏度
    pedometer.peak_threshold = 8000;      // 降低峰值阈值
    pedometer.noise_threshold = 2000;     // 降低噪声阈值
    pedometer.min_step_interval = 100;    // 缩短最小步间隔到100毫秒
    pedometer.last_step_time = 0;
    pedometer.step_state = 0;             // 初始状态：等待波峰
    
    printf("Simple pedometer initialized with high sensitivity\r\n");
    
    // 加载保存的步数数据
    // Steps_Load();
}

/**
 * @brief 计算三轴加速度的合加速度
 * @param ax X轴加速度
 * @param ay Y轴加速度
 * @param az Z轴加速度
 * @return 合加速度值
 */
static short calculate_magnitude(short ax, short ay, short az)
{
    // 计算三轴加速度的合加速度 (sqrt(ax^2 + ay^2 + az^2))
    // 为避免浮点运算，我们使用近似算法
    long magnitude_sq = (long)ax * ax + (long)ay * ay + (long)az * az;
    
    // 简单的整数平方根近似算法
    if (magnitude_sq <= 0) return 0;
    
    short magnitude = 0;
    short low = 0;
    short high = 0x7FFF; // 32767
    
    // 二分查找法求平方根
    while (high - low > 1) {
        short mid = (low + high) / 2;
        long sq = (long)mid * mid;
        if (sq <= magnitude_sq) {
            low = mid;
        } else {
            high = mid;
        }
    }
    
    magnitude = low;
    return magnitude;
}

/**
 * @brief 更新计步器状态并检测步数
 * @param ax X轴加速度
 * @param ay Y轴加速度
 * @param az Z轴加速度
 * @return 当前步数
 */
unsigned long simple_pedometer_update(short ax, short ay, short az)
{
    // 计算合加速度
    short magnitude = calculate_magnitude(ax, ay, az);
    
    // 获取当前时间（简化处理，使用固定间隔）
    static unsigned long current_time = 0;
    current_time += 100; // 假设每100ms调用一次
    
    // 简单的峰值检测算法
    short diff = magnitude - pedometer.last_acceleration;
    
    // 状态机检测步数
    if (pedometer.step_state == 0) {
        // 等待波峰状态
        if (diff < -pedometer.noise_threshold && 
            magnitude < pedometer.last_acceleration - pedometer.noise_threshold) {
            // 检测到波峰
            if (pedometer.last_acceleration > pedometer.peak_threshold) {
                // 波峰足够大
                pedometer.step_state = 1; // 转换到等待波谷状态
            }
        }
    } else {
        // 等待波谷状态
        if (diff > pedometer.noise_threshold && 
            magnitude > pedometer.last_acceleration + pedometer.noise_threshold) {
            // 检测到波谷
            // 检查时间间隔，避免重复计数
            if (current_time - pedometer.last_step_time > pedometer.min_step_interval) {
                // 计为一步
                g_step_count++;
                pedometer.last_step_time = current_time;
                printf("Step detected! Total steps: %lu\r\n", g_step_count);
                
                // 每100步或达到特定步数时保存一次
                if (g_step_count % 100 == 0) {
                    // Steps_Save();
                }
            }
            pedometer.step_state = 0; // 回到等待波峰状态
        }
    }
    
    // 更新上一次的加速度值
    pedometer.last_acceleration = magnitude;
    
    return g_step_count;
}

/**
 * @brief 获取当前步数
 * @return 当前步数
 */
unsigned long simple_pedometer_get_steps(void)
{
    return g_step_count;
}

/**
 * @brief 重置计步器
 */
void simple_pedometer_reset(void)
{
    g_step_count = 0;
    pedometer.last_acceleration = 0;
    pedometer.last_step_time = 0;
    pedometer.step_state = 0;
    printf("Simple pedometer reset\r\n");
    
    // 重置后立即保存
    // Steps_Save();
}
