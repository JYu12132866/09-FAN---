#include "rt_param.h"
#include "main.h"
#include "error.h"
#include <string.h>
#include "stmflash.h"
#include "modectl.h"
#include "c381.h"
#include <stdbool.h>
#include "uart_protocal.h"

// 存储参数格式
#define CFG_VER_MAJOR   1   
#define CFG_VER_MINOR   0



// 显示模式字符串表
const char dispModeStrTab[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "2D",
    "3D_in",
    "3D_ext",
    "Smear",
    "3D_st",
    "Low_lat",
};

// 色温字符串表
const char colorTempStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "9300K",
    "7500K",
    "6500K",
    "Custom",
};

// 颜色空间字符串表
const char colorSpaceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "BT709",
    "DCI-P3",
    "BT2020",
    "Origin",
};

// 安装模式字符串表
const char installationModeStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "Front",
    "Rear",
    "Ceil_front",
    "Ceil_rear",
};


// 输入源字符串表
const char inputSourceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "Auto",
    "VGA",
    "DP_1",
    "HDMI1",
    "DVI",
    "HDMI2",
    "DP_2",
    "HD_BASE",
};


// 输入颜色格式字符串表
const char inputColorFormatStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "RGB",
    "YPbPr",
    "Auto",
};

// 输入色域字符串表
const char inputColorSpaceStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "RGB",
    "YUV"
};

// 伽玛字符串表
const char gammaStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "1.8",
    "2.0",
    "2.2",
    "2.4",
    "Off",
};

// 背景颜色字符串表
const char backgroundColorStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "Red",
    "Green",
    "Blue",
    "Black",
    "White",
};

// 语言字符串表
const char languageStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "English",
    "Chinese",
};

// 宽高比字符串表
const char aspectRatioStrTable[M_TAB_SIZE][M_TAB_STR_SIZE] = {
    "Full",
    "Auto",
    "4:3",
    "16:9",
    "16:10",
    "Origin",
};

// 硬件信息，不存储flash，当前只有pjlink 在用
HW_Info hw_info = {
    .projector_name = ProjectorName,
    .menufacture_name = "AVIC118_GuoHua",
    .product_name = ProductName,
    .serial_num = "20240101999",
    .mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

// 运行时信息， 不更新进入flash
RunTimeInfo run_time_info = {0};

// 记录的参数
rt_param_s  rt_param = {0};

bool g_save_flag = false;       // falsh 回写任务flag

/* 工厂模式开关： 
* 0：关   
* 1：等级1，集成商 & 集成部使用,提供OEM配置参数修改权限
* 2：等级2，生产 & 售后使用使用，提供生产相关配置参数修改权限
* 3：等级3，研发使用 ，最高权限 */
uint8_t factoryModel = 0;

/* Functions----------------------------------------------------------------------------------------------------------------*/

/**
 * @brief 获取参数描述字符串
 * 
 * @param param_id 参数ID
 * @return const char* 参数描述字符串
 */
const char* get_param_desc_str(uint8_t param_id, const char table[][M_TAB_STR_SIZE])
{
    if(param_id < M_TAB_SIZE)
    {
        return table[param_id];
    }
    return NULL;
}


/**
 * @brief 从字符串获取参数ID
 * @param param_value 
 * @return 
 */
int get_param_id_by_value(const char* param_value, const char table[][M_TAB_STR_SIZE])
{
    for(int i = 0; i < M_TAB_SIZE; i++)
    {
        if(strcmp(table[i], param_value) == 0)
        {
            return i;
        }
    }
    return -1; // 未找到
}


/**
 * @brief 从Flash中读取运行时参数
 * 
 */
void read_rt_param_from_flash(void)
{   
    uint8_t rstWrBack = 0; //参数回写标记

    // 读取flash中的参数
    STMFLASH_Read(ADDR_FLASH_SECTOR_7, (uint32_t*)(&rt_param), sizeof(rt_param_s)/sizeof(uint32_t));
    printf("Read runtime param, flash_erase_cnt=[%d]\r\n", rt_param.flash_erase_cnt);
    
    //判断flash中参数是否第一次烧录，或版本更新后缺失，填补参数
    {   
        //检查更新运行时参数
        if(rt_param.work_mode == 0xffffffff) 
        {
            // 默认运行时配置
            rt_param.work_mode = WORK_MODE_NORMAL;
            rt_param.video_in_chan = VIDEO_INPUT_SRC_VGA;
            rt_param.health_report_en = ON;
            rt_param.health_report_format = 1;
            rt_param.health_report_period = 30;
            rt_param.c381_mode = C381_MODE_4K;
            rt_param.shift_motor_model = SHIFT_MOTOR_DZ_24BYJ48_A14; //默认是新电机型号
            rstWrBack = 1 ;

        }
    
        // 检查更新硬件信息字段为0或0xff时
        if( rt_param.menufacture_name[0] == 0 || rt_param.menufacture_name[0] == 0xff)  
        {
            // 默认硬件信息
            strcpy(rt_param.menufacture_name, "AVIC_118_GuoHua");
            strcpy(rt_param.product_name, ProductName);
            strcpy(rt_param.product_model, "NP52SL");
            strcpy(rt_param.serial_num, "20250000000");
            strcpy(rt_param.manufacture_date, "2025-01-01-00-00-00");
            strcpy(rt_param.display_model, "NP52SL");
            strcpy(rt_param.display_sn, "20258888888");
            strcpy(rt_param.display_date, "2025-01-01-00-00-00");
            strcpy(rt_param.mac, "00:00:00:00:00:00");
            rt_param.lamp_num = 0;
            rt_param.lamp_time = 0;
        }

        
        //检察更新3D 延迟信号参数
        if((uint32_t)rt_param.sync_3d_delay_arg == 0xffffffff || rt_param.sync_3d_delay_arg < -8.192f || rt_param.sync_3d_delay_arg > 8.192f)
        {
            rt_param.sync_3d_delay_arg = -1.2f;  // 根据实测，该参数可以使3D输入输出信号同步
            rstWrBack = 1 ;
        }
        
        // 检查更新待机上报使能参数
        if(rt_param.xml_standby_report_en != 0 && rt_param.xml_standby_report_en != 1)
        {
            rt_param.xml_standby_report_en = 0; // 默认值为0，表示待机状态下不进行XML上报
            rstWrBack = 1 ;
        }
        
        // 检查更新上下电记录参数
        if(rt_param.power_on_count == 0xFFFFFFFF || rt_param.power_off_count == 0xFFFFFFFF)
        {
            rt_param.power_on_count = 0;
            rt_param.power_off_count = 0;
            rstWrBack = 1;
        }

        if(rt_param.health_report_period < 5 )  // 兼容旧版本格式，如果是1，2，3，4 分别对应 30s,45s,50s,60s
        {
            if(rt_param.health_report_period == 1)
                rt_param.health_report_period = 30;
            else if (rt_param.health_report_period == 2)
                rt_param.health_report_period = 45;
            else if (rt_param.health_report_period == 3)
                rt_param.health_report_period = 60;
            else if (rt_param.health_report_period == 4)
                rt_param.health_report_period = 90;
        }
    }

    // 刷新存储参数刷新给运行时模块(兼容层)
    {
        Source_Stat = (VIDEO_INPUT_SRC_)(rt_param.video_in_chan);
        OE_lock = (rt_param.lens_lock == 1) ? TRUE : FALSE; // 镜头锁定状态
    }   
    
    //如果有更新参数，则直接重新保存一遍
    if(rstWrBack)
    {
        printf("uninited rt param, Reset.\n");
        rt_param_reset(); // 重置参数  
        save_rt_param_to_flash(); // 保存参数
    }

    #if 1
    // 打印所有参数
    printf("========================================\n");
    printf("Runtime Parameters Loaded from Flash\n");
    printf("========================================\n");
    
    // [Section 1: Runtime Config]
    printf("[Runtime Config]\n");
    printf("  flash_erase_cnt: %u\n", rt_param.flash_erase_cnt);
    printf("  version: %u.%u\n", rt_param.version[0], rt_param.version[1]);
    printf("  work_mode: %d\n", rt_param.work_mode);
    printf("  video_in_chan: %d (%s)\n", rt_param.video_in_chan, 
           rt_param.video_in_chan < M_TAB_SIZE ? inputSourceStrTable[rt_param.video_in_chan] : "Unknown");
    printf("  health_report_en: %d\n", rt_param.health_report_en);
    printf("  health_report_format: %d\n", rt_param.health_report_format);
    printf("  health_report_period: %d\n", rt_param.health_report_period);
    printf("  c381_mode: %d\n", rt_param.c381_mode);
    printf("  shift_motor_model: 0x%02X\n", rt_param.shift_motor_model);
    printf("  sync_3d_delay_arg: %.3f\n", rt_param.sync_3d_delay_arg);
    printf("  xml_standby_report_en: %d\n", rt_param.xml_standby_report_en);
    printf("  power_on_count: %d\n", rt_param.power_on_count);
    printf("  power_off_count: %d\n", rt_param.power_off_count);
    
    printf("  display_mode: %u (%s)\n", rt_param.display_mode,
           rt_param.display_mode < M_TAB_SIZE ? dispModeStrTab[rt_param.display_mode] : "Unknown");
    printf("  color_space: %u (%s)\n", rt_param.color_space,
           rt_param.color_space < M_TAB_SIZE ? colorSpaceStrTable[rt_param.color_space] : "Unknown");
    printf("  color_temp: %u (%s)\n", rt_param.color_temp,
           rt_param.color_temp < M_TAB_SIZE ? colorTempStrTable[rt_param.color_temp] : "Unknown");
    
    printf("  laser_en: %u\n", rt_param.laser_en);
    printf("  laser_lvl: %u\n", rt_param.laser_lvl);
    printf("  ir_en: %u\n", rt_param.ir_en);
    printf("  ir_lvl: %u\n", rt_param.ir_lvl);
    printf("  ir_id: %u\n", rt_param.ir_id);
    
    printf("  installation_mode: %u (%s)\n", rt_param.installation_mode,
           rt_param.installation_mode < M_TAB_SIZE ? installationModeStrTable[rt_param.installation_mode] : "Unknown");
    printf("  high_altitude: %u\n", rt_param.high_altitude);
    printf("  input_source: %u (%s)\n", rt_param.input_source,
           rt_param.input_source < M_TAB_SIZE ? inputSourceStrTable[rt_param.input_source] : "Unknown");
    printf("  input_color_format: %u (%s)\n", rt_param.input_color_format,
           rt_param.input_color_format < M_TAB_SIZE ? inputColorFormatStrTable[rt_param.input_color_format] : "Unknown");
    printf("  input_color_space: %u (%s)\n", rt_param.input_color_space,
           rt_param.input_color_space < M_TAB_SIZE ? inputColorSpaceStrTable[rt_param.input_color_space] : "Unknown");
    
    printf("  contrast: %u\n", rt_param.contrast);
    printf("  sharpness: %u\n", rt_param.sharpness);
    printf("  brightness: %u\n", rt_param.brightness);
    printf("  gamma: %u (%s)\n", rt_param.gamma,
           rt_param.gamma < M_TAB_SIZE ? gammaStrTable[rt_param.gamma] : "Unknown");
    printf("  background_color: %u\n", rt_param.background_color);
    
    printf("  lens_lock: %u\n", rt_param.lens_lock);
    printf("  alarm_en: %u\n", rt_param.alarm_en);
    printf("  alarm_tip: %u\n", rt_param.alarm_tip);
    printf("  no_input_poweroff: %u\n", rt_param.no_input_poweroff);
    printf("  power_auto_boot: %u\n", rt_param.power_auto_boot);
    printf("  aspect_ratio: %u\n", rt_param.aspect_ratio);
    printf("  language: %u\n", rt_param.language);
    printf("  eco_mode_en: %u\n", rt_param.eco_mode_en);
    
    printf("  HSG_param[21]: ");
    for(int i = 0; i < 21; i++) {
        printf("%u ", rt_param.HSG_param[i]);
    }
    printf("\n");
    
    printf("  RGB_gain[3]: ");
    for(int i = 0; i < 3; i++) {
        printf("%u ", rt_param.RGB_gain[i]);
    }
    printf("\n");
    
    printf("  indep_color[6]: ");
    for(int i = 0; i < 6; i++) {
        printf("%u ", rt_param.indep_color[i]);
    }
    printf("\n");
    
    printf("  ip_addr: %u.%u.%u.%u\n", 
           rt_param.ip_addr[0], rt_param.ip_addr[1], 
           rt_param.ip_addr[2], rt_param.ip_addr[3]);
    printf("  mask: %u.%u.%u.%u\n", 
           rt_param.mask[0], rt_param.mask[1], 
           rt_param.mask[2], rt_param.mask[3]);
    printf("  gateway: %u.%u.%u.%u\n", 
           rt_param.gateway[0], rt_param.gateway[1], 
           rt_param.gateway[2], rt_param.gateway[3]);
    printf("  report_ip: %u.%u.%u.%u\n", 
           rt_param.report_ip[0], rt_param.report_ip[1], 
           rt_param.report_ip[2], rt_param.report_ip[3]);
    printf("  report_port: %u\n", rt_param.report_port);
    
    printf("  fw_version1(MCU): %u.%u.%u\n", 
           rt_param.fw_version1[0], rt_param.fw_version1[1], rt_param.fw_version1[2]);
    printf("  fw_version2(MStar): %u.%u.%u\n", 
           rt_param.fw_version2[0], rt_param.fw_version2[1], rt_param.fw_version2[2]);
    printf("  fw_version3(DLP): %u.%u.%u\n", 
           rt_param.fw_version3[0], rt_param.fw_version3[1], rt_param.fw_version3[2]);
    
    // [Section 2: Hardware Info]
    printf("\n[Hardware Info]\n");
    printf("  menufacture_name: %s\n", rt_param.menufacture_name);
    printf("  product_name: %s\n", rt_param.product_name);
    printf("  product_model: %s\n", rt_param.product_model);
    printf("  serial_num: %s\n", rt_param.serial_num);
    printf("  manufacture_date: %s\n", rt_param.manufacture_date);
    printf("  display_model: %s\n", rt_param.display_model);
    printf("  display_sn: %s\n", rt_param.display_sn);
    printf("  display_date: %s\n", rt_param.display_date);
    printf("  mac: %s\n", rt_param.mac);
    printf("  lamp_num: %d\n", rt_param.lamp_num);
    printf("  lamp_time: %d\n", rt_param.lamp_time);
    
    printf("========================================\n");
    #endif 
}


/**
 * @brief 真正把flash参数存储至Flash，定时器中调用。
 * @param void   
 */
void do_save_rt_param_to_flash(void)
{
    if(!g_save_flag) 
        return;
    
    //判断是否需要更新flash
    rt_param_s  temp_param = {0};
    STMFLASH_Read(ADDR_FLASH_SECTOR_7, (uint32_t*)(&temp_param), sizeof(rt_param_s)/sizeof(uint32_t));
    if(memcmp(&temp_param, &rt_param, sizeof(rt_param_s)) == 0)
    {
        printf("No need save rt_param\r\n");
        g_save_flag = 0;  // 清除标志，避免重复触发
        return;
    }
    //更新flash擦除计数
    if(rt_param.flash_erase_cnt == 0xffffffff) 
    {
        rt_param.flash_erase_cnt = 0; 
    }
    rt_param.flash_erase_cnt++;
    //写入Flash
    STMFLASH_Write(ADDR_FLASH_SECTOR_7, (uint32_t*)(&rt_param), sizeof(rt_param_s)/sizeof(uint32_t));
    printf("Save rt_param\r\n");
    g_save_flag = 0;
}
    

/**
 * @brief 存储运行时参数至Flash,仅置标记，定时器中调用。
 * 
 */
void save_rt_param_to_flash(void)
{
    g_save_flag = 1; 
    return;
}

uint8_t read_hw_version(void)
{
    static uint8_t init_flag = 0;
    static uint8_t hw_version = 0;

    //根据三个GPIO上下拉，对应3个BIT位，读取PCBA版本
    // PB2 PB3 PF4  MSB -> LSB
    if(init_flag == 0)
    {
        if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_SET)
        {
            hw_version |= 0x04;
        }
        if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET)
        {
            hw_version |= 0x02;
        }
        if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_4) == GPIO_PIN_SET)
        {
            hw_version |= 0x01;
        }        
        init_flag = 1;

        printf("System Board Version = b'%d%d%d\r\n", 
            (hw_version >> 2) & 0x1, 
            (hw_version >> 1) & 0x1, 
            hw_version & 0x1);
    }
    return hw_version;
}



int rt_param_reset()
{
    rt_param.work_mode = WORK_MODE_NORMAL;
    rt_param.video_in_chan = VIDEO_INPUT_SRC_HDMI_1;
    rt_param.health_report_en = ON;
    rt_param.health_report_format = 1;
    rt_param.health_report_period = 30;
    rt_param.c381_mode = C381_MODE_4K;
    rt_param.xml_standby_report_en = ON;
    rt_param.display_mode = 0;
    rt_param.color_space = 1;
    rt_param.color_temp = 0;
    rt_param.laser_en = 1;
    rt_param.laser_lvl = 99;
    rt_param.ir_en = 1;
    rt_param.ir_lvl = 100;
    rt_param.ir_id = 0;
    rt_param.installation_mode = 0;
    rt_param.high_altitude = 0;
    rt_param.input_source = 3;
    rt_param.input_color_format = 0;
    rt_param.input_color_space = 0;
    rt_param.contrast = 50;
    rt_param.sharpness = 50;
    rt_param.brightness = 50;
    rt_param.gamma = 4;
    rt_param.background_color = 2;
    rt_param.lens_lock = 0;
    rt_param.alarm_en = 1;
    rt_param.alarm_tip = 0;
    rt_param.no_input_poweroff = 0;
    rt_param.power_auto_boot = 0;
    rt_param.aspect_ratio = 0;
    rt_param.language = 1;
    memset(rt_param.HSG_param, 50, sizeof(rt_param.HSG_param));
    memset(rt_param.RGB_gain, 50, sizeof(rt_param.RGB_gain));
    memset(rt_param.indep_color, 50, sizeof(rt_param.indep_color));
    rt_param.ip_addr[0] = 192;
    rt_param.ip_addr[1] = 168;
    rt_param.ip_addr[2] = 0;
    rt_param.ip_addr[3] = 20;
    rt_param.mask[0] = 255;
    rt_param.mask[1] = 255;
    rt_param.mask[2] = 255;
    rt_param.mask[3] = 0;
    rt_param.gateway[0] = 192;
    rt_param.gateway[1] = 168;
    rt_param.gateway[2] = 0;
    rt_param.gateway[3] = 1;
    rt_param.report_ip[0] = 192;
    rt_param.report_ip[1] = 168;
    rt_param.report_ip[2] = 0;
    rt_param.report_ip[3] = 100;
    rt_param.report_port = 8080;
    rt_param.fw_version1[0] = VERSION_MAJOR;
    rt_param.fw_version1[1] = VERSION_MINOR;
    rt_param.fw_version1[2] = VERSION_PATCH;
    rt_param.fw_version2[0] = 0;
    rt_param.fw_version2[1] = 0;
    rt_param.fw_version2[2] = 0;
    rt_param.fw_version3[0] = 0;
    rt_param.fw_version3[1] = 0;
    rt_param.fw_version3[2] = 0;
    rt_param.eco_mode_en = 0;
    rt_param.laser_bright[0] = 1024;
    rt_param.laser_bright[1] = 1024;
    rt_param.laser_bright[2] = 1024;
    save_rt_param_to_flash();
    return 0 ;
}


int rt_param_sync_to_mstar(void)
{
    typedef struct
    {
        uint8_t hw_version;
        uint8_t display_mode;
        uint8_t color_space;
        uint8_t color_temp;
        uint8_t laser_en;
        uint8_t laser_lvl;
        uint8_t ir_en;
        uint8_t ir_lvl;
        uint8_t ir_id;
        uint8_t installation_mode;
        uint8_t high_altitude;
        uint8_t input_source;
        uint8_t input_color_format;
        uint8_t input_color_space;
        uint8_t contrast;
        uint8_t sharpness;
        uint8_t brightness;
        uint8_t gamma;
        uint8_t background_color;
        uint8_t lens_lock;
        uint8_t alarm_en;
        uint8_t alarm_tip;
        uint8_t no_input_poweroff;
        uint8_t power_auto_boot;
        uint8_t aspect_ratio;
        uint8_t language;
        uint8_t HSG_param[21];
        uint8_t RGB_gain[3];
        uint8_t indep_color[6];
        uint8_t ip_addr[4];
        uint8_t mask[4];
        uint8_t gateway[4];
        uint8_t report_ip[4];
        uint16_t report_port;
        uint8_t fw_version1[3];     //MCU固件版本
        uint8_t fw_version2[3];     //MStar固件版本
        uint8_t fw_version3[3];     //DLP固件版本
        uint8_t eco_mode_en;        //节能模式开关
        uint16_t laser_bright[3];   //激光亮度等级 R/G/B 0-1024
    } rt_param_sync_to_mstar_t;

    rt_param_sync_to_mstar_t mstar_param = {0};
    mstar_param.hw_version = read_hw_version();
    mstar_param.display_mode = rt_param.display_mode;
    mstar_param.color_space = rt_param.color_space;
    mstar_param.color_temp = rt_param.color_temp;
    mstar_param.laser_en = rt_param.laser_en;
    mstar_param.laser_lvl = rt_param.laser_lvl;
    mstar_param.ir_en = rt_param.ir_en;
    mstar_param.ir_lvl = rt_param.ir_lvl;
    mstar_param.ir_id = rt_param.ir_id;
    mstar_param.installation_mode = rt_param.installation_mode;
    mstar_param.high_altitude = rt_param.high_altitude;
    mstar_param.input_source = rt_param.input_source;
    mstar_param.input_color_format = rt_param.input_color_format;
    mstar_param.input_color_space = rt_param.input_color_space;
    mstar_param.contrast = rt_param.contrast;
    mstar_param.sharpness = rt_param.sharpness;
    mstar_param.brightness = rt_param.brightness;
    mstar_param.gamma = rt_param.gamma;
    mstar_param.background_color = rt_param.background_color;
    mstar_param.lens_lock = rt_param.lens_lock;
    mstar_param.alarm_en = rt_param.alarm_en;
    mstar_param.alarm_tip = rt_param.alarm_tip;
    mstar_param.no_input_poweroff = rt_param.no_input_poweroff;
    mstar_param.power_auto_boot = rt_param.power_auto_boot;
    mstar_param.aspect_ratio = rt_param.aspect_ratio;
    mstar_param.language = rt_param.language;
    mstar_param.ip_addr[0] = rt_param.ip_addr[0];
    mstar_param.ip_addr[1] = rt_param.ip_addr[1];
    mstar_param.ip_addr[2] = rt_param.ip_addr[2];
    mstar_param.ip_addr[3] = rt_param.ip_addr[3];
    mstar_param.mask[0] = rt_param.mask[0];
    mstar_param.mask[1] = rt_param.mask[1];
    mstar_param.mask[2] = rt_param.mask[2];
    mstar_param.mask[3] = rt_param.mask[3];
    mstar_param.gateway[0] = rt_param.gateway[0];
    mstar_param.gateway[1] = rt_param.gateway[1];
    mstar_param.gateway[2] = rt_param.gateway[2];
    mstar_param.gateway[3] = rt_param.gateway[3];
    mstar_param.report_ip[0] = rt_param.report_ip[0];
    mstar_param.report_ip[1] = rt_param.report_ip[1];
    mstar_param.report_ip[2] = rt_param.report_ip[2];
    mstar_param.report_ip[3] = rt_param.report_ip[3];
    mstar_param.report_port = rt_param.report_port;
    memcpy(mstar_param.HSG_param, rt_param.HSG_param, sizeof(rt_param.HSG_param));
    memcpy(mstar_param.RGB_gain, rt_param.RGB_gain, sizeof(rt_param.RGB_gain));
    memcpy(mstar_param.indep_color, rt_param.indep_color, sizeof(rt_param.indep_color));
    mstar_param.fw_version1[0] = VERSION_MAJOR;
    mstar_param.fw_version1[1] = VERSION_MINOR;
    mstar_param.fw_version1[2] = VERSION_PATCH;
    mstar_param.eco_mode_en = rt_param.eco_mode_en;
    mstar_param.laser_bright[0] = rt_param.laser_bright[0];
    mstar_param.laser_bright[1] = rt_param.laser_bright[1];
    mstar_param.laser_bright[2] = rt_param.laser_bright[2];

    #define MST_CMD_PRARM_SYNC 0X08 // 参数同步指令
    appSendUartAsync(MST_CMD_PRARM_SYNC, (uint8_t*)&mstar_param, sizeof(rt_param_sync_to_mstar_t));
    return 0;
}
