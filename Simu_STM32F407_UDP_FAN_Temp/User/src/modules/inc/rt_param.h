#ifndef _RT_PARAM_H
#define _RT_PARAM_H
#include <stdint.h>

// 定义硬件型号
#define SHIFT_MOTOR_OLD                 0x11        //旧马达：型号
#define SHIFT_MOTOR_DZ_24BYJ48_A14      0x22        //新马达：DZ-24BYJ48-A14

// 3D延迟功能支持的最低硬件版本 (0-7, 3位二进制)
#define MIN_HW_VERSION_FOR_3D_DELAY     1

/**
 * @brief 键值对结构体，用于存储键值对数据
 */



// clang-format off
#pragma pack(1) // 1字节对齐
// 运行时需要保存和读取的参数
typedef struct
{
    /* 运行时配置参数------------------------------------------------ */
    uint16_t flash_erase_cnt;       // flash 擦除次数记录
    uint8_t version[2];             // 参数版本号（未使用）
    int work_mode;                  // 工作模式记录
    int video_in_chan;              // 输入通道记录
    int health_report_en;           // 状态上报开关
    int health_report_format;       // 状态上报格式
    int health_report_period;       // 状态上报周期
    int c381_mode;                  // C381工作模式
    int shift_motor_model;          // 移轴镜头马达型号
    float sync_3d_delay_arg;        // 3D同步延迟设置参数
    int xml_standby_report_en;      // XML报文待机后依旧上报开关
    int power_on_count;             // MStar 上电事件累计次数
    int power_off_count;            // MStar 正常关机事件累计次数
    uint8_t debug_area[32];         // 用于debug的数据记录区域
    uint8_t display_mode;           // 显示模式           
    uint8_t color_space;            // 颜色空间
    uint8_t color_temp;             // 色温
    uint8_t laser_en;               // 激光开关
    uint8_t laser_lvl;              // 激光亮度等级
    uint8_t ir_en;                  // 红外开关
    uint8_t ir_lvl;                 // 红外亮度等级
    uint8_t ir_id;                  // 红外ID
    uint8_t installation_mode;      // 安装模式 
    uint8_t high_altitude;          // 高海拔模式
    uint8_t input_source;           // 输入源
    uint8_t input_color_format;     // 输入颜色格式
    uint8_t input_color_space;      // 输入色域
    uint8_t contrast;               // 对比度
    uint8_t sharpness;              // 清晰度
    uint8_t brightness;             // 背景亮度
    uint8_t gamma;                  // 伽玛
    uint8_t background_color;       // 背景色
    uint8_t lens_lock;              // 镜头锁定
    uint8_t alarm_en;               // 自检报警开关
    uint8_t alarm_tip;              // 告警提示开关
    uint8_t no_input_poweroff;      // 无输入关机
    uint8_t power_auto_boot;        // 上电自动开机
    uint8_t aspect_ratio;           // 宽高比
    uint8_t language;               // 语言
    uint8_t HSG_param[21];          // HSG颜色设置参数 RGBCYMW[H/S/G]
    uint8_t RGB_gain[3];            // RGB增益设置参数
    uint8_t indep_color[6];         // 独立颜色设置参数 RGBCYM
    uint8_t ip_addr[4];             // 本机IP地址
    uint8_t mask[4];                // 本机子网掩码
    uint8_t gateway[4];             // 本机网关地址
    uint8_t report_ip[4];           // 上报目标IP地址
    uint16_t report_port;           // 上报目标端口
    uint8_t fw_version1[3];         // MCU固件版本
    uint8_t fw_version2[3];         // MStar固件版本
    uint8_t fw_version3[3];         // DLP固件版本
    uint8_t eco_mode_en;            // 节能模式开关
    uint16_t laser_bright[3];       // 激光亮度等级 R/G/B 0-1024
    uint8_t rsv[58];                // 保留字段

    /* 结构体内部分界，硬件相关参数----------------------------------- */
    char menufacture_name[32];      // 制造商名称
    char product_name[32];          // 设备名称
    char product_model[32];         // 产品型号
    char serial_num[32];            // 产品序列号
    char manufacture_date[32];      // 出厂日期, 格式为YYYY-MM-DD-HH-MM-SS
    char display_model[32];         // 显示型号 
    char display_sn[32];            // 显示序列号
    char display_date[32];          // 显示出厂日期, 格式为YYYY-MM-DD-HH-MM-SS
    char mac[16];                   // MAC地址
    int lamp_num;                   // 光源（激光/灯泡）数量
    int lamp_time;                  // 光源（激光/灯泡）使用时间
    uint8_t hw_info_rsv[128];       // 保留字段

    /* 其它参数 rsv--------------------------------------------------*/
    uint8_t other_rsv[128];         // 保留字段, 后续存储其它信息

} rt_param_s;                       

// 设备硬件状态信息
typedef struct
{
    char projector_name[32];   // 设备名称
    char menufacture_name[32]; // 制造商名称
    char product_name[32];     // 产品名称
    uint8_t serial_num[11];    // 序列号
    uint8_t mac[6];            // MAC地址
    int lamp_num;              // 灯泡数量
    int lamp_time;             // 灯泡使用时间
} HW_Info;

typedef struct
{
    uint8_t current_display_mode;     // 当前显示模式
    uint16_t current_resolution[2];   // 当前分辨率  W x H
    uint16_t recommend_resolution[2]; // 推荐分辨率  W x H
    uint16_t current_fps;             // 当前帧率
    uint8_t freeze_status;            // 冻结状态
} RunTimeInfo;

#pragma pack()
// clang-format on

extern rt_param_s rt_param;
extern HW_Info hw_info;
extern RunTimeInfo run_time_info;

extern uint8_t factoryModel;

// 参数与描述字符串对应查找表
#define M_TAB_SIZE      8
#define M_TAB_STR_SIZE  12
extern const char dispModeStrTab[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char colorTempStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char colorSpaceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char installationModeStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char inputSourceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char inputColorFormatStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char inputColorSpaceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char gammaStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char backgroundColorStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char languageStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];
extern const char aspectRatioStrTable[M_TAB_SIZE][M_TAB_STR_SIZE];

const char* get_param_desc_str(uint8_t param_id, const char table[][M_TAB_STR_SIZE]);
int get_param_id_by_value(const char* param_value, const char table[][M_TAB_STR_SIZE]);

void read_rt_param_from_flash(void);
void save_rt_param_to_flash(void);
void do_save_rt_param_to_flash(void);

int rt_param_reset(void);
int rt_param_sync_to_mstar(void);

/**
 * @brief 读取硬件版本,根据GPIO上下拉，读取PCBA版本
 */
uint8_t read_hw_version(void);

#endif // _RT_PARAM_H
