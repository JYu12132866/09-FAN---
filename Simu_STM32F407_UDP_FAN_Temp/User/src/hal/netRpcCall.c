/******************************************************* 
* @file netRpcCall.c
* @author Primo Wu (wuxy101@avic.com)
* @brief 
* @version 1.0
* @date 2024-07-16
* 
* @copyright Copyright (c) 2024 .
* 
*******************************************************/
#include "netRpcCall.h"
#include <string.h>
#include "libs/cJSON/cJSON.h"
#include "hal/uart_protocal.h"
#include "main.h"
#include "udp_demo.h"
#include "stdlib.h"
#include "i2c.h"
#include "modectl.h"
#include "error.h"
#include "rt_param.h"
#include "base64.h"
#include "logger.h"
#include "delay3D.h"
#include "pj_hw_def.h"
#include "power_monitor.h"
#include "util.h"
#include "healthMgr.h"

// clang-format off

#define RCP_20_PROTOCAL_VERSION              1.0f

#define FACTORY_MODE_PASSWORD_LVL_1          (219)          // 工厂模式密码等级1 for OEM use
#define FACTORY_MODE_PASSWORD_LVL_2          (617)          // 工厂模式密码等级2 for Production use
#define FACTORY_MODE_PASSWORD_LVL_3          (318)          // 工厂模式密码等级3 for Development use


//Json RPC 2.0协议 错误码定义
#define JSONRPC20_PARSE_ERROR               (-32700)
#define JSONRPC20_INVALID_QUEST             (-32600)
#define JSONRPC20_METHOND_NOT_FOUND         (-32601)
#define JSONRPC20_INVALID_PARAM             (-32602)
#define JSONRPC20_INTERNAL_ERROR            (-32603)
#define JSONRPC20_SERVER_ERROR              (-32000)


typedef int (*rpc_func_ptr)(cJSON *, cJSON *, cJSON **);        // RPC函数指针定义


/*******************************************************
* @brief RPC函数索引结构体定义
* 
*******************************************************/
#pragma pack(1)
typedef struct  
{
    rpc_func_ptr    funcPtr;
    char            name[24];
}RpcFunc;
#pragma pack()


/* RPC 功能函数前置声明*/
/********************************************************/
int rpc_common_power(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_video_background(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_lens_focus(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_lens_zoom(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_lens_shift(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_lens_lock(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_color_brightness(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_color_contrast(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_color_sharpness(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_color_gamma(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_commu_mac(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_misc_testpattern(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_misc_hialt(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_light_laser_switch(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_light_ir_switch(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_light_ir_brightness(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_model(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_serial(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_date(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_version(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_all(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_temperature(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_error(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_worktime(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_hsg(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_laser_brightness(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_hwcfg(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_prd_info(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_route_mstar(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_3d_delay_set(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_3d_delay_get(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_xml_standby_report(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_error_poweroff(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_info_hw_version(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_com_ip(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_report_confg(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_rt_param(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_protocol_version(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_display_info(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_sys_reset(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_quick_ops(cJSON *params, cJSON *id, cJSON **resJson);
int rpc_fty_mode_auth(cJSON *params, cJSON *id, cJSON **resJson);




/*******************************************************
* @brief  - RCP函数索引注册表 外部调用;
*         - 实时性不高，使用字符串索引的方式查找对应函数
*******************************************************/
RpcFunc rpcFuncTab[] = {
    { rpc_protocol_version,                   "protocol"            },
    { rpc_common_power,                       "comm.power"          },
    { rpc_video_background,                   "video.background"    },
    { rpc_lens_focus,                         "lens.focus"          },
    { rpc_lens_zoom,                          "lens.zoom"           },
    { rpc_lens_shift,                         "lens.shift"          },
    { rpc_lens_lock,                          "lens.lock"           },
    { rpc_color_brightness,                   "color.bright"        },
    { rpc_color_contrast,                     "color.ctrs"          },
    { rpc_color_sharpness,                    "color.sharp"         },
    { rpc_color_gamma,                        "color.gamma"         },
    { rpc_commu_mac,                          "commu_mac"           },
    { rpc_misc_testpattern,                   "misc.testpattern"    },
    { rpc_misc_hialt,                         "misc.hialt"          },
    { rpc_light_laser_switch,                 "light.laser.switch"  },
    { rpc_light_ir_switch,                    "light.ir.switch"     },
    { rpc_light_ir_brightness,                "light.ir.bright"     },
    { rpc_info_model,                         "info.model"          },
    { rpc_info_serial,                        "info.serial"         },
    { rpc_info_date,                          "info.date"           },
    { rpc_info_version,                       "info.version"        },
    { rpc_info_temperature,                   "info.temp"           },
    { rpc_info_error,                         "info.error"          },
    { rpc_info_all,                           "info.all"            },
    { rpc_fty_mode_auth,                      "fty.mode.auth"       },
    { rpc_fty_worktime,                       "fty.worktime",       },
    { rpc_fty_hsg,                            "fty.hsg"             },
    { rpc_fty_laser_brightness,               "fty.laser.bright"    },
    { rpc_fty_hwcfg,                          "fty.hwcfg"           },
    { rpc_fty_prd_info,                       "fty.prd.info"        },     
    { rpc_route_mstar,                        "route.mstar"         },   
#if (Projector_Model == LP72BSL_NP52SLC_NP52SL)
    { rpc_3d_delay_set,                       "3d.delay.set"        },
    { rpc_3d_delay_get,                       "3d.delay.get"        },
#endif
    { rpc_xml_standby_report,                 "report.standby.en"   },
    { rpc_info_error_poweroff,                "info.error_poweroff" },
    { rpc_info_hw_version,                    "info.hw.version"     },
    { rpc_com_ip,                             "com.ip"              },
    { rpc_report_confg,                       "com.packet"          },
    { rpc_rt_param,                           "rt.param"            },
    { rpc_sys_reset,                          "system.reset"        },
    { rpc_quick_ops,                          "quick.ops"           },
    { rpc_fty_display_info,                   "fty.display.info"    },
};
// clang-format on

/*******************************************************
* @brief 函数
* 
* @param funcName 
* @return int 
*******************************************************/
rpc_func_ptr findRpcFunction(const char *funcName)
{
    // 查找str对应的函数指针
    uint8_t arraySize = sizeof(rpcFuncTab) / sizeof(RpcFunc);
    for (size_t i = 0; i < arraySize; i++)
    {
        if (0 == strcmp(funcName, rpcFuncTab[i].name))
        {
            if (rpcFuncTab[i].funcPtr != NULL)
                return rpcFuncTab[i].funcPtr;
        }
    }
    return NULL;
}


/*******************************************************
* @brief 构建rpc2.0 错误包
* 
* @param errCode    错误码
* @param id         通信id
* @param retJson    返回数据指针
* @return int       返回值
*******************************************************/
int constructRpc20ErrJson(int errCode, int id, cJSON **retJson)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *error = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddItemToObject(root, "error", error);
    cJSON_AddNumberToObject(error, "code", errCode);

    uint8_t noIdFlag = 0;

    switch (errCode)
    {
    case JSONRPC20_PARSE_ERROR:
        cJSON_AddStringToObject(error, "message", "Parse error");
        noIdFlag = 1;
        break;
    case JSONRPC20_INTERNAL_ERROR:
        cJSON_AddStringToObject(error, "message", "Internal error");
        break;
    case JSONRPC20_INVALID_QUEST:
        cJSON_AddStringToObject(error, "message", "Invalid Request");
        noIdFlag = 1;
        break;
    case JSONRPC20_METHOND_NOT_FOUND:
        cJSON_AddStringToObject(error, "message", "Method not found");
        break;
    case JSONRPC20_INVALID_PARAM:
        cJSON_AddStringToObject(error, "message", "Invalid params");
        break;
    default:
        cJSON_AddStringToObject(error, "message", "Server error");
        break;
    }

    if (noIdFlag)
        cJSON_AddItemToObject(root, "id", cJSON_CreateNull());
    else
        cJSON_AddNumberToObject(root, "id", id);

    *retJson = root;
    return 0;
    // return root;
}


/*******************************************************
* @brief 构建rpc2.0协议函数调用结果包
* 
* @param result         返回结果的json node
* @param id             通信id
* @param retJson        返回json包的指针
* @return int           返回值
*******************************************************/
int constructRpc20Result(cJSON* result, int id, cJSON** retJson)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddItemToObject(root, "result", result);
    cJSON_AddNumberToObject(root, "id", id);
    *retJson = root;
    return 0 ;
}


/*******************************************************
* @brief  网络rpc包解析函数
* 
* @param rpcCmdStr  rpc json 包字符串
* @return int 
*******************************************************/
int netRpcParse(const char *rpcCmdStr)
{
    cJSON *jsonrpc = NULL;
    cJSON *id = NULL;
    cJSON *method = NULL;
    cJSON *params = NULL;
    cJSON *retJson = NULL;
    char *retJsonStr = NULL;
    rpc_func_ptr rpc_cb = NULL;

    cJSON *json = cJSON_Parse(rpcCmdStr);
    if (json == NULL)
    {
        printf("JSON parse fail\n");
        constructRpc20ErrJson(JSONRPC20_PARSE_ERROR, 0, &retJson);
        goto done;
    }

    // 解析所有参数
    jsonrpc = cJSON_GetObjectItemCaseSensitive(json, "jsonrpc");
    id      = cJSON_GetObjectItemCaseSensitive(json, "id");
    method  = cJSON_GetObjectItemCaseSensitive(json, "method");
    params  = cJSON_GetObjectItemCaseSensitive(json, "params");

    // 判断参数是否为空或格式异常，全部归纳为invalid request
    if (!cJSON_IsString(jsonrpc) || !cJSON_IsString(method) || !cJSON_IsNumber(id) || !cJSON_IsObject(params))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, &retJson);
        goto done;
    }

    printf("jsonrpc: %s\n", jsonrpc->valuestring);
    printf("method: %s\n", method->valuestring);
    printf("id: %d\n", id->valueint);

    if (0 != strcmp(jsonrpc->valuestring, "2.0"))
    {
        printf("json rpc version invalid: %s\n", jsonrpc->valuestring);
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, &retJson);
        goto done;
    }

    // 查找函数并执行,获得响应包
    rpc_cb = findRpcFunction(method->valuestring);
    if (NULL == rpc_cb)
    {
        // 未找到对应的方法，method not found
        constructRpc20ErrJson(JSONRPC20_METHOND_NOT_FOUND, id->valueint, &retJson);
        goto done;
    }

    rpc_cb(params, id, &retJson);

done:
    // 返回执行结果, 错误包或结果包
    // retJsonStr = cJSON_Print(retJson);
    retJsonStr = cJSON_PrintUnformatted(retJson);
    udp_send_remote((uint8_t *)retJsonStr, strlen(retJsonStr));

    // 释放cJSON对象
    free(retJsonStr);
    cJSON_Delete(json);
    cJSON_Delete(retJson);
    return 0;
}

/*********************************************************************************************************************************************/
/* RPC 功能函数 */

/*******************************************************
* @brief 获取协议版本
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_protocol_version(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *result = cJSON_CreateObject();
    char version_str[16];
    snprintf(version_str, sizeof(version_str), "%.1f", (double)RCP_20_PROTOCAL_VERSION);
    cJSON_AddRawToObject(result, "version", version_str);
    constructRpc20Result(result, id->valueint, resJson);
    return 0;
}



/*******************************************************
* @brief 电源控制RPC函数
* 
* @param params 
* @param id 
* @return cJSON* 
*******************************************************/
int rpc_common_power(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(params, "state");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(state))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_POWER, CMDTYPE_SET, cJSON_IsTrue(state) ? PWR_ON : PWR_OFF};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        printf("ret = %d\n", ret);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
    
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_POWER, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddBoolToObject(result, "state", (ackData[0] == PWR_ON) ? true : false);
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            printf("retlen = %d\n", retLen);
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/*******************************************************
* @brief rpc 获取mac地址
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_commu_mac(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();

        char macStr[20] = {0};
        sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x",
            lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);

        cJSON_AddStringToObject(result, "mac", macStr);
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/*******************************************************
* @brief rpc函数，设置无信号背景颜色
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_video_background(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *color = cJSON_GetObjectItemCaseSensitive(params, "color");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(color))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        // 构建消息包
        uint8_t data[3] = {MST_CMD_BACKGROUND, CMDTYPE_SET, 0};
        if(0 == strcmp(color->valuestring,"red"))
            data[2] = 0;
        else if(0 == strcmp(color->valuestring,"green"))
            data[2] = 1;
        else if(0 == strcmp(color->valuestring,"blue"))
            data[2] = 2;
        else if(0 == strcmp(color->valuestring,"write"))
            data[2] = 3;
        else if(0 == strcmp(color->valuestring,"black"))
            data[2] = 4;
        else 
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }     
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));

        rt_param.background_color = data[2];
        save_rt_param_to_flash();

        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}


/*******************************************************
* @brief 控制镜头聚焦/虚焦
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_lens_focus(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *dir = cJSON_GetObjectItemCaseSensitive(params, "dir");
    cJSON *step = cJSON_GetObjectItemCaseSensitive(params, "step");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(dir) || !cJSON_IsNumber(step))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        // 构建消息包
        uint8_t data[3] = {MST_CMD_FOCUS, CMDTYPE_SET, 0};

        if(0 == strcmp(dir->valuestring, "futher"))
        {
            lensCtrlStep(I2C_OE_Focus, LENS_FOCUS_FUTHER);
            data[2] = CMD_FOCUS_FUTHER;
        }
        else if(0 == strcmp(dir->valuestring, "closer"))
        {
            lensCtrlStep(I2C_OE_Focus, LENS_FOCUS_CLOSER);
            data[2] = CMD_FOCUS_CLOSER;
        }
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 控制镜头zoom放大缩小
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_lens_zoom(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *dir = cJSON_GetObjectItemCaseSensitive(params, "dir");
    cJSON *step = cJSON_GetObjectItemCaseSensitive(params, "step");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(dir) || !cJSON_IsNumber(step))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_ZOOM, CMDTYPE_SET, 0};

        if(0 == strcmp(dir->valuestring, "plus"))
        {
            lensCtrlStep(I2C_OE_Zoom, LENS_ZOOM_PLUS);
            data[2] = CMD_ZOOM_PLUS;
        }
        else if(0 == strcmp(dir->valuestring, "minus"))
        {
            lensCtrlStep(I2C_OE_Zoom, LENS_ZOOM_MINUS);
            data[2] = CMD_ZOOM_MINUS;
        }
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 控制镜头位移
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_lens_shift(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *dir = cJSON_GetObjectItemCaseSensitive(params, "dir");
    cJSON *step = cJSON_GetObjectItemCaseSensitive(params, "step");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(dir) || !cJSON_IsNumber(step))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();
          // 构建消息包
        uint8_t data[3] = {MST_CMD_SHIFT, CMDTYPE_SET, 0};

        if(0 == strcmp(dir->valuestring, "up"))
        {
            // lensCtrlStep(I2C_OE_UD, LENS_SHIFT_UP);
            data[2] = CMD_SHIFT_UP;
        }
        else if(0 == strcmp(dir->valuestring, "down"))
        {
            // lensCtrlStep(I2C_OE_UD, LENS_SHIFT_DOWN);
            data[2] = CMD_SHIFT_DOWN;
        }
        else if(0 == strcmp(dir->valuestring, "left"))
        {
            // lensCtrlStep(I2C_OE_LR, LENS_SHIFT_LEFT);
            data[2] = CMD_SHIFT_LEFT;
        }
        else if(0 == strcmp(dir->valuestring, "right"))
        {
            // lensCtrlStep(I2C_OE_LR, LENS_SHIFT_RIGHT);
            data[2] = CMD_SHIFT_RIGHT;
        }
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if(0 == strcmp(mode->valuestring, "reset"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        lensCtrlStep(I2C_OE_Rst, 0);
        uint8_t data[3] = {MST_CMD_SHIFT, CMDTYPE_SET, CMD_RESET};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else{
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 控制镜头锁定
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_lens_lock(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *lock = cJSON_GetObjectItemCaseSensitive(params, "lock");


    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(lock))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        lensCtrlStep(I2C_OE_Lock,  cJSON_IsTrue(lock) ? LENS_LOCK : LENS_UNLOCK);

        uint8_t data[3] = {MST_CMD_LENS_LOCK, CMDTYPE_SET, cJSON_IsTrue(lock) ? CMD_ON : CMD_OFF};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);

        rt_param.lens_lock = cJSON_IsTrue(lock) ? 1 : 0;
        OE_lock = (rt_param.lens_lock == 1) ? TRUE : FALSE;
        save_rt_param_to_flash();

        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));

        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 显示亮度调整函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_color_brightness(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *brtVal = cJSON_GetObjectItemCaseSensitive(params, "value");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(brtVal))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_BRIGHTNESS, CMDTYPE_SET, brtVal->valueint};
        
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        rt_param.brightness = data[2];
        save_rt_param_to_flash();

        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_BRIGHTNESS, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "value", ackData[0]);
            rt_param.brightness = ackData[0];
            save_rt_param_to_flash();
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 对比度参数设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_color_contrast(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *ctrsVal = cJSON_GetObjectItemCaseSensitive(params, "value");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(ctrsVal))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_CONTRAST, CMDTYPE_SET, ctrsVal->valueint};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);

        rt_param.contrast = data[2];
        save_rt_param_to_flash();

        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_CONTRAST, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "value", ackData[0]);
            rt_param.contrast = ackData[0];
            save_rt_param_to_flash();
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 清晰度设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_color_sharpness(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *sharpVal = cJSON_GetObjectItemCaseSensitive(params, "value");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(sharpVal))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_SHARPNESS, CMDTYPE_SET, sharpVal->valueint};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        rt_param.sharpness = data[2];
        save_rt_param_to_flash();
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_SHARPNESS, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "value", ackData[0]);
            rt_param.sharpness = ackData[0];
            save_rt_param_to_flash();
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief Gamma参数设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_color_gamma(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *gammaVal = cJSON_GetObjectItemCaseSensitive(params, "value");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(gammaVal))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
         // 构建消息包
        uint8_t data[3] = {MST_CMD_GAMMA, CMDTYPE_SET, gammaVal->valueint};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        rt_param.gamma = data[2];
        save_rt_param_to_flash();
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_GAMMA, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "value", ackData[0]);
            rt_param.gamma = ackData[0];
            save_rt_param_to_flash();
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 测图设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_misc_testpattern(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *tp = cJSON_GetObjectItemCaseSensitive(params, "pattern");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(tp))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        // 构建消息包
        uint8_t data[3] = {MST_CMD_TESTPATTERN, CMDTYPE_SET, 0};
        if(0==strcmp(tp->valuestring,"exit"))
            data[2] = 0;
        else if(0==strcmp(tp->valuestring,"red"))
            data[2] = 1;
        else if(0==strcmp(tp->valuestring,"green"))
            data[2] = 2;
        else if(0==strcmp(tp->valuestring,"blue"))
            data[2] = 3;
        else if(0==strcmp(tp->valuestring,"black"))
            data[2] = 4;
        else if(0==strcmp(tp->valuestring,"write"))
            data[2] = 5;
        else if(0==strcmp(tp->valuestring,"gray"))
            data[2] = 6;
        else if(0==strcmp(tp->valuestring,"checkboard"))
            data[2] = 7;
        else if(0==strcmp(tp->valuestring,"whitegrid"))
            data[2] = 8;
        else 
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }     
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));

        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};

/*******************************************************
* @brief 高海拔模式开关
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_misc_hialt(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(params, "state");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(state))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();

        modectl_switch_mode( cJSON_IsTrue(state) ? MODECTL_CMD_HI_ALT_ON : MODECTL_CMD_HI_ALT_OFF);

        // 构建消息包
        uint8_t data[3] = {MST_CMD_MODE_HI_ALT, CMDTYPE_SET, cJSON_IsTrue(state) ? CMD_ON : CMD_OFF};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);

        rt_param.high_altitude = cJSON_IsTrue(state) ? 1 : 0;
        save_rt_param_to_flash();

        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
    
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
  
        cJSON_AddBoolToObject(result, "state", modeclt_is_HiAlt_On() ? true : false);
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 可见光激光光源开关
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_light_laser_switch(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(params, "state");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(state))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        uint8_t data[3] = {MST_CMD_LIGHT_LASER_SWITCH, CMDTYPE_SET, cJSON_IsTrue(state) ? CMD_ON : CMD_OFF};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        rt_param.laser_en = cJSON_IsTrue(state) ? 1 : 0;
        save_rt_param_to_flash();
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));

        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 红外光源开关函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_light_ir_switch(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(params, "state");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(state))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();

        uint8_t data[3] = {MST_CMD_LIGHT_IR_SWITCH, CMDTYPE_SET, cJSON_IsTrue(state) ? CMD_ON : CMD_OFF};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        rt_param.ir_en = cJSON_IsTrue(state) ? 1 : 0;
        save_rt_param_to_flash();
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));

        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 红外光源亮度设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_light_ir_brightness(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *brtVal = cJSON_GetObjectItemCaseSensitive(params, "value");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(brtVal))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_LIGHT_IR_BRIGHTNESS, CMDTYPE_SET, brtVal->valueint};
        
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);

        rt_param.ir_lvl = data[2];
        save_rt_param_to_flash();
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_LIGHT_IR_BRIGHTNESS, CMDTYPE_GET};
        uint8_t ackData[2] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 2, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "value", ackData[0]);
            rt_param.ir_lvl = ackData[0];
            save_rt_param_to_flash();
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 型号获取
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int  rpc_info_model(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
   
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
        return -1;
    }
  
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
     
        /*
        // 构建消息包
        uint8_t data[2] = {MST_CMD_INFO_MODEL, CMDTYPE_GET};
        uint8_t ackData[32] = {0};
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 32, 500);
        if (retLen > 0)
        {
            cJSON_AddStringToObject(result, "model", (const char*)ackData);
            
            snprintf(rt_param.display_model, sizeof(rt_param.display_model), "%s", (const char*)ackData);

            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
        */

        cJSON_AddStringToObject(result, "model", rt_param.display_model);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;

    }
    // else if(0 == strcmp(mode->valuestring, "set") && factoryModel >= 1) // 工厂模式大于等于1，则支持设置修改
    // {
    //     cJSON *model = cJSON_GetObjectItemCaseSensitive(params, "model");
    //     if (!cJSON_IsString(model)) {
    //         constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
    //         return -1;
    //     }

    //     snprintf(rt_param.display_model, sizeof(rt_param.display_model), "%s", model->valuestring);
    //     save_rt_param_to_flash();

    //     cJSON *result = cJSON_CreateObject();
    //     cJSON_AddBoolToObject(result, "return", true);
    //     constructRpc20Result(result, id->valueint, resJson);
    //     return 0;
    // }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 序列号参数获取
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_info_serial(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
   
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
  
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();

        /*
        // 构建消息包
        uint8_t data[2] = {MST_CMD_INFO_SERIAL, CMDTYPE_GET};
        uint8_t ackData[32] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 32, 500);
        if (retLen > 0)
        {
            char serial[16] = {0};
            for (int i = 0; i < retLen; i++) {
                serial[i] = ackData[i] + '0'; // 将数字转换为对应的 ASCII 字符
            }
            cJSON_AddStringToObject(result, "serial", (const char*)serial);
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
        */

        cJSON_AddStringToObject(result, "serial", rt_param.serial_num);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;

    }
    // else if(0 == strcmp(mode->valuestring, "set") && factoryModel >= 1) // 工厂模式大于等于1，则支持设置修改
    // {
    //     cJSON *serial = cJSON_GetObjectItemCaseSensitive(params, "serial");
    //     if (!cJSON_IsString(serial)) {
    //         constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
    //         return -1;
    //     }

    //     snprintf(rt_param.display_sn, sizeof(rt_param.display_sn), "%s", serial->valuestring);
    //     save_rt_param_to_flash();

    //     cJSON *result = cJSON_CreateObject();
    //     cJSON_AddBoolToObject(result, "return", true);
    //     constructRpc20Result(result, id->valueint, resJson);
    //     return 0;
    // }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};

/**
 * @brief 获取生产日期信息
 * @param params 
 * @param id 
 * @param resJson 
 * @return 
 */
int rpc_info_date(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        cJSON_AddStringToObject(result, "date", rt_param.display_date);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if(0 == strcmp(mode->valuestring, "set") && factoryModel >= 1) // 工厂模式大于等于1，则支持设置修改
    {
        cJSON *date = cJSON_GetObjectItemCaseSensitive(params, "date");
        if (!cJSON_IsString(date)) {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
            return -1;
        }
        snprintf(rt_param.display_date, sizeof(rt_param.display_date), "%s", date->valuestring);
        save_rt_param_to_flash();

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
		return 0;
};

/*******************************************************
* @brief 版本号获取
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_info_version(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
   
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
  
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_INFO_FW_VERSION, CMDTYPE_GET};
        uint8_t ackData[8] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 32, 500);

        char fw_mcu[16] = {0};
        char fw_mstar[16] = {0};
        char fw_dlp[16] = {0};
        sprintf(fw_mcu,     "V%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        sprintf(fw_mstar,   "V%d.%d.%d", ackData[0], ackData[1], ackData[2]);
        sprintf(fw_dlp,     "V%d.%d.%d", ackData[3], ackData[4], ackData[5]);

        printf("fw_mcu: %s\n", fw_mcu);
        printf("fw_mstar: %s\n", fw_mstar);
        printf("fw_dlp: %s\n", fw_dlp);

        cJSON_AddStringToObject(result, "fw_mcu", fw_mcu);
        cJSON_AddStringToObject(result, "fw_mstar", fw_mstar);
        cJSON_AddStringToObject(result, "fw_dlp", fw_dlp);
        
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 温度信息获取
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_info_temperature(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
   
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
  
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();

        printf("ADS_Temp_Data[0]: %d\n", (int)ADS_Temp_Data[0]);
        printf("ADS_Temp_Data[1]: %d\n", (int)ADS_Temp_Data[1]);
        printf("ADS_Temp_Data[2]: %d\n", (int)ADS_Temp_Data[2]);
        printf("ADS_Temp_Data[3]: %d\n", (int)ADS_Temp_Data[3]);
        printf("ADS_Temp_Data[4]: %d\n", (int)ADS_Temp_Data[4]);
        printf("ADS_Temp_Data[6]: %d\n", (int)ADS_Temp_Data[6]);
        printf("ADS_Temp_Data[7]: %d\n", (int)ADS_Temp_Data[7]);
        printf("DMD_Temp: %d\n", DMD_Temp);
        
        cJSON_AddNumberToObject(result, "R1", (int)(ADS_Temp_Data[0]));
        cJSON_AddNumberToObject(result, "R2", (int)(ADS_Temp_Data[1]));
        cJSON_AddNumberToObject(result, "G1", (int)(ADS_Temp_Data[2]));
        cJSON_AddNumberToObject(result, "G2", (int)(ADS_Temp_Data[3]));
        cJSON_AddNumberToObject(result, "B1", (int)(ADS_Temp_Data[4]));
        cJSON_AddNumberToObject(result, "B2", (int)(ADS_Temp_Data[6]));
        cJSON_AddNumberToObject(result, "ENV", (int)(ADS_Temp_Data[7]));
        cJSON_AddNumberToObject(result, "DMD", DMD_Temp);
        
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


int rpc_info_error(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if(0 == strcmp(mode->valuestring, "get"))
    {
        uint32_t fanErr = getFanErrCode();
        uint32_t tempErr = getTempErrCode();
        cJSON *result = cJSON_CreateObject();
        cJSON *error = cJSON_CreateArray();  
        cJSON_AddItemToArray(error, cJSON_CreateNumber(fanErr));
        cJSON_AddItemToArray(error, cJSON_CreateNumber(tempErr));
        cJSON_AddItemToObject(result, "error", error);
        constructRpc20Result(result, id->valueint, resJson);
    }
    else
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }

    return 0;
}

/*******************************************************
* @brief 获取所有信息
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_info_all(cJSON *params, cJSON *id, cJSON **resJson){
  
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    // 参数检查
    if (!cJSON_IsString(mode)){
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
  
    // get分支
    if (0 == strcmp(mode->valuestring, "get")){
        cJSON *result = cJSON_CreateObject();
        cJSON_AddStringToObject(result, "menufacture", rt_param.menufacture_name);
        cJSON_AddStringToObject(result, "name", rt_param.product_name);
        cJSON_AddStringToObject(result, "model", rt_param.display_model);
        cJSON_AddStringToObject(result, "sn", rt_param.display_sn);
        cJSON_AddStringToObject(result, "date", rt_param.display_date);

        char versionStr[16] = {0};
        cJSON* fwVersion =  cJSON_CreateArray();
        snprintf(versionStr, sizeof(versionStr), "V%d.%d.%d", rt_param.fw_version1[0], rt_param.fw_version1[1], rt_param.fw_version1[2]);
        cJSON_AddItemToArray(fwVersion, cJSON_CreateString(versionStr));
        snprintf(versionStr, sizeof(versionStr), "V%d.%d.%d", rt_param.fw_version2[0], rt_param.fw_version2[1], rt_param.fw_version2[2]);
        cJSON_AddItemToArray(fwVersion, cJSON_CreateString(versionStr));
        snprintf(versionStr, sizeof(versionStr), "V%d.%d.%d", rt_param.fw_version3[0], rt_param.fw_version3[1], rt_param.fw_version3[2]);
        cJSON_AddItemToArray(fwVersion, cJSON_CreateString(versionStr));
        cJSON_AddItemToObject(result, "fw_version", fwVersion);
        
        constructRpc20Result(result, id->valueint, resJson);          // 返回结果
        return 0;
    }
    else{
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};



/*******************************************************
* @brief 工作时间获取函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_fty_worktime(cJSON *params, cJSON *id, cJSON **resJson){
   
    if(factoryModel < 2) // 工厂模式小于1，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
   
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_FTY_WORKTIME, CMDTYPE_GET};
        uint8_t ackData[6] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 6, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "worktime", *(uint16_t*)ackData);
            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 工厂设置，HSG颜色参数设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_fty_hsg(cJSON *params, cJSON *id, cJSON **resJson){
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *color = cJSON_GetObjectItemCaseSensitive(params, "color");
    cJSON *attr = cJSON_GetObjectItemCaseSensitive(params, "attr");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(params, "value");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(value) || !cJSON_IsString(color) || !cJSON_IsString(attr))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();
        // 构建消息包

        uint8_t data[6] = {MST_CMD_FTY_HSG, CMDTYPE_SET, 0 , 0 , 0 , 0 };
        if(0 == strcmp(color->valuestring, "r"))
            data[2] = 0x1;
        else if(0 == strcmp(color->valuestring, "g")) 
            data[2] = 0x2;
        else if(0 == strcmp(color->valuestring, "b")) 
            data[2] = 0x3;
        else if(0 == strcmp(color->valuestring, "c")) 
            data[2] = 0x4;
        else if(0 == strcmp(color->valuestring, "y")) 
            data[2] = 0x5;
        else if(0 == strcmp(color->valuestring, "m")) 
            data[2] = 0x6;
        else if(0 == strcmp(color->valuestring, "w")) 
            data[2] = 0x7;
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        if(0 == strcmp(attr->valuestring, "r") || 0 == strcmp(attr->valuestring, "hue"))
            data[3] = 0x1;
        else if(0 == strcmp(attr->valuestring, "g") || 0 == strcmp(attr->valuestring, "satu")) 
            data[3] = 0x2;
        else if(0 == strcmp(attr->valuestring, "b") || 0 == strcmp(attr->valuestring, "gain")) 
            data[3] = 0x3;
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        *(uint16_t*)(&data[4]) = value->valueint;

        // 发送mstar
        int ret = appSendUartMsg(data, 5, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_FTY_HSG, CMDTYPE_GET};
        uint8_t ackData[64] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 64, 500);
        if (retLen > 0)
        {
            cJSON *R = cJSON_CreateObject();
            cJSON_AddNumberToObject(R, "H", *(uint8_t*)(&ackData[0]));
            cJSON_AddNumberToObject(R, "S", *(uint8_t*)(&ackData[7]));
            cJSON_AddNumberToObject(R, "G", *(uint8_t*)(&ackData[14]));
            cJSON_AddItemToObject(result, "R", R);

            cJSON *G = cJSON_CreateObject();
            cJSON_AddNumberToObject(G, "H", *(uint8_t*)(&ackData[1]));
            cJSON_AddNumberToObject(G, "S", *(uint8_t*)(&ackData[8]));
            cJSON_AddNumberToObject(G, "G", *(uint8_t*)(&ackData[15]));
            cJSON_AddItemToObject(result, "G", G);
            
            cJSON *B = cJSON_CreateObject();
            cJSON_AddNumberToObject(B, "H", *(uint8_t*)(&ackData[2]));
            cJSON_AddNumberToObject(B, "S", *(uint8_t*)(&ackData[9]));
            cJSON_AddNumberToObject(B, "G", *(uint8_t*)(&ackData[16]));
            cJSON_AddItemToObject(result, "B", B);
            
            cJSON *C = cJSON_CreateObject();
            cJSON_AddNumberToObject(C, "H", *(uint8_t*)(&ackData[3]));
            cJSON_AddNumberToObject(C, "S", *(uint8_t*)(&ackData[10]));
            cJSON_AddNumberToObject(C, "G", *(uint8_t*)(&ackData[17]));
            cJSON_AddItemToObject(result, "C", C);
            
            cJSON *Y = cJSON_CreateObject();
            cJSON_AddNumberToObject(Y, "H", *(uint8_t*)(&ackData[4]));
            cJSON_AddNumberToObject(Y, "S", *(uint8_t*)(&ackData[11]));
            cJSON_AddNumberToObject(Y, "G", *(uint8_t*)(&ackData[18]));
            cJSON_AddItemToObject(result, "Y", Y);
            
            cJSON *M = cJSON_CreateObject();
            cJSON_AddNumberToObject(M, "H", *(uint8_t*)(&ackData[5]));
            cJSON_AddNumberToObject(M, "S", *(uint8_t*)(&ackData[12]));
            cJSON_AddNumberToObject(M, "G", *(uint8_t*)(&ackData[19]));
            cJSON_AddItemToObject(result, "M", M);
            
            cJSON *W = cJSON_CreateObject();
            cJSON_AddNumberToObject(W, "R", *(uint8_t*)(&ackData[6]));
            cJSON_AddNumberToObject(W, "G", *(uint8_t*)(&ackData[13]));
            cJSON_AddNumberToObject(W, "B", *(uint8_t*)(&ackData[20]));
            cJSON_AddItemToObject(result, "W", W);

            memcpy(rt_param.HSG_param, ackData, 21);
            save_rt_param_to_flash();

            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else if(0 == strcmp(mode->valuestring, "reset"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_FTY_HSG, CMDTYPE_SET, CMD_RESET};
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        
        memset(rt_param.HSG_param, 50, 21);
        save_rt_param_to_flash();
        
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};


/*******************************************************
* @brief 工厂设置，激光光源亮度分量设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_fty_laser_brightness(cJSON *params, cJSON *id, cJSON **resJson){
    if(factoryModel < 2) // 工厂模式小于2，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *source = cJSON_GetObjectItemCaseSensitive(params, "source");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(params, "value");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(value) || !cJSON_IsString(source))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[5] = {MST_CMD_FTY_LASER_BRIGHT, CMDTYPE_SET, 0 , 0 , 0};
        if(0 == strcmp(source->valuestring, "r"))
            data[2] = 0x1;
        else if(0 == strcmp(source->valuestring, "g")) 
            data[2] = 0x2;
        else if(0 == strcmp(source->valuestring, "b")) 
            data[2] = 0x3;
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }
        *(uint16_t*)(&data[3]) = value->valueint;

        // 发送mstar
        int ret = appSendUartMsg(data, 5, true, NULL, 0, 500);

        rt_param.laser_bright[data[2]-1] = *(uint16_t*)(&data[3]);
        save_rt_param_to_flash();

        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        constructRpc20Result(result, id->valueint, resJson);
        return -1;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[2] = {MST_CMD_FTY_LASER_BRIGHT, CMDTYPE_GET};
        uint8_t ackData[8] = {0};
   
        // 发送mstar
        int retLen = appSendUartMsg(data, 2, true, ackData, 8, 500);
        if (retLen > 0)
        {
            cJSON_AddNumberToObject(result, "R", *(uint16_t*)(&ackData[0]));
            cJSON_AddNumberToObject(result, "G", *(uint16_t*)(&ackData[2]));
            cJSON_AddNumberToObject(result, "B", *(uint16_t*)(&ackData[4]));

            rt_param.laser_bright[0] = *(uint16_t*)(&ackData[0]);
            rt_param.laser_bright[1] = *(uint16_t*)(&ackData[2]);
            rt_param.laser_bright[2] = *(uint16_t*)(&ackData[4]);
            save_rt_param_to_flash();

            // 返回结果
            constructRpc20Result(result, id->valueint, resJson);
            return 0;
        }
        else
        {
            // 返回异常执行结果，定义为server err
            constructRpc20ErrJson(JSONRPC20_SERVER_ERROR, id->valueint, resJson);
            return -1;
        }
    }
    else if(0 == strcmp(mode->valuestring, "reset"))
    {
        cJSON *result = cJSON_CreateObject();
        // 构建消息包
        uint8_t data[3] = {MST_CMD_FTY_LASER_BRIGHT, CMDTYPE_SET, CMD_RESET};
   
        // 发送mstar
        int ret = appSendUartMsg(data, 3, true, NULL, 0, 500);
        cJSON_AddBoolToObject(result,"return", (ret >= 0 ? true : false));
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};

/*******************************************************
* @brief 硬件型号参数设置
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_fty_hwcfg(cJSON *params, cJSON *id, cJSON **resJson){
    if(factoryModel < 2) // 工厂模式小于2，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *hw = cJSON_GetObjectItemCaseSensitive(params, "hw");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(params, "value");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsString(hw) || !cJSON_IsString(value))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }
        cJSON *result = cJSON_CreateObject();
        
        if(0 == strcmp(hw->valuestring, "motor"))
        {
            if(0 == strcmp(value->valuestring, "new"))
            {
                rt_param.shift_motor_model = SHIFT_MOTOR_DZ_24BYJ48_A14;
                save_rt_param_to_flash();
            }
            else if(0 == strcmp(value->valuestring, "old"))
            {
                rt_param.shift_motor_model = SHIFT_MOTOR_OLD;
                save_rt_param_to_flash();
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                return -1;
            }
        }
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }
        cJSON_AddBoolToObject(result,"return",  true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
};

/*******************************************************
* @brief 批量设置、获取生产信息：包括生产日期、产品型号、产品序列号等
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_fty_prd_info(cJSON *params, cJSON *id, cJSON **resJson){

    if(factoryModel < 2) // 工厂模式小于2，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    if(0 == strcmp(mode->valuestring, "set")){
        cJSON *date = cJSON_GetObjectItemCaseSensitive(params, "date");
        cJSON *serial = cJSON_GetObjectItemCaseSensitive(params, "serial");

        if (!cJSON_IsString(date) || !cJSON_IsString(serial)) {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
            return -1;
        }
        
        // 设置产品序列号和出厂日期， 生产使用
        snprintf(rt_param.serial_num, sizeof(rt_param.serial_num), "%s", serial->valuestring);
        snprintf(rt_param.manufacture_date, sizeof(rt_param.manufacture_date), "%s", date->valuestring);
        save_rt_param_to_flash();

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if(strcmp(mode->valuestring, "get") == 0)
    {
        cJSON *result = cJSON_CreateObject();
        cJSON_AddStringToObject(result, "name", rt_param.product_name);      // 产品名称
        cJSON_AddStringToObject(result, "model", rt_param.product_model);    // 产品型号
        cJSON_AddStringToObject(result, "serial", rt_param.serial_num);      // 产品序列号
        cJSON_AddStringToObject(result, "date", rt_param.manufacture_date);  // 出厂日期
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
		return 0;
}

int rpc_fty_display_info(cJSON *params, cJSON *id, cJSON **resJson){
    
    if(factoryModel < 1) // 工厂模式小于1，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    
    if(0 == strcmp(mode->valuestring, "set"))
    {

        cJSON *model = cJSON_GetObjectItemCaseSensitive(params, "model");
        if(model != NULL && cJSON_IsString(model))
        {
            snprintf(rt_param.display_model, sizeof(rt_param.display_model), "%s", model->valuestring);
            uint8_t payload[32] = {0};
            payload[0] = PARAM_ID_DISP_MODEL;
            strcpy((char*)(payload + 1), rt_param.display_model);
            appSendUartAsync(MST_CMD_PARAM_SET, payload, strlen(rt_param.display_model) + 2);
            HAL_Delay(100);
        }
        
        cJSON *serial = cJSON_GetObjectItemCaseSensitive(params, "serial");
        if(serial != NULL && cJSON_IsString(serial))
        {
            snprintf(rt_param.display_sn, sizeof(rt_param.display_sn), "%s", serial->valuestring);
            uint8_t payload[32] = {0};
            payload[0] = PARAM_ID_DISP_SN;
            strcpy((char*)(payload + 1), rt_param.display_sn);
            appSendUartAsync(MST_CMD_PARAM_SET, payload, strlen(rt_param.display_sn) + 2);
            HAL_Delay(100);
        }
        
        cJSON *date = cJSON_GetObjectItemCaseSensitive(params, "date");
        if(date != NULL && cJSON_IsString(date))
        {
            snprintf(rt_param.display_date, sizeof(rt_param.display_date), "%s", date->valuestring);
            uint8_t payload[32] = {0};
            payload[0] = PARAM_ID_DISP_DATE;
            strcpy((char*)(payload + 1), rt_param.display_date);
            appSendUartAsync(MST_CMD_PARAM_SET, payload, strlen(rt_param.display_date) + 2);
            HAL_Delay(100);
        }
        
        // 保存参数到flash
        save_rt_param_to_flash();

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else 
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/*******************************************************
* @brief Mstar消息透传接口
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_route_mstar(cJSON *params, cJSON *id, cJSON **resJson){

    cJSON *value = cJSON_GetObjectItemCaseSensitive(params, "value");

    // 参数检查
    if (!cJSON_IsString(value))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
        return -1;
    }

    cJSON *result = cJSON_CreateObject();

    // 将value这个字符串转成uint8_t数组
    const char* pInputData = value->valuestring;
    size_t input_length = strlen(pInputData);

    size_t output_length = 0;
    unsigned char* pData = base64_decode(pInputData, input_length, &output_length);
    
    //打印转换后的数据，用于debug
    printf("parsed data length: %d\n", output_length);
    for(int i = 0; i < output_length; i++){
        printf("%x ", pData[i]);
    }
    printf("\n");

    // 发送mstar
    int ret = appSendUartRawData(pData, output_length, false, NULL, 0, 500);
    
    free(pData);

    cJSON_AddBoolToObject(result, "return",  ret >= 0 ? true : false);
    constructRpc20Result(result, id->valueint, resJson);
    return 0;

}; 

/*******************************************************
* @brief 设置3D延迟RPC函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_3d_delay_set(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *delay = cJSON_GetObjectItemCaseSensitive(params, "delay");
    
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsNumber(delay))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        // 检查硬件版本是否支持3D延迟功能
        if (!is_3d_delay_supported())
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        
        // 调用3D延迟设置函数
        float delay_ms = (float)delay->valuedouble;
        delay_ms = roundf(delay_ms * 1000.0f) / 1000.0f;
        set_delay3D(delay_ms);
        
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/*******************************************************
* @brief 读取3D延迟RPC函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_3d_delay_get(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        // 检查硬件版本是否支持3D延迟功能
        if (!is_3d_delay_supported())
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }
        
        cJSON *result = cJSON_CreateObject();
        
        // 调用3D延迟获取函数
        float delay_ms = 0.0f;
        get_delay3D(&delay_ms);

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.3f", delay_ms);
        cJSON_AddRawToObject(result, "delay", buffer);
        
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/**
 * @brief MST芯片透传数据通知函数
 * 
 * @param data 透传数据指针
 * @param data_len 数据长度
 * @return int 执行结果
 */
int mstar_transparent_notify(uint8_t* data, uint8_t data_len)
{
    if (data == NULL || data_len == 0) {
        log_warn("Invalid transparent data");
        return -1;
    }
    
    // 创建JSON通知对象
    cJSON *root = cJSON_CreateObject();
    cJSON *params = cJSON_CreateObject();
    
    // 构建JSON-RPC 2.0通知格式
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddStringToObject(root, "method", "mstar.notify");
    cJSON_AddItemToObject(root, "params", params);
    
    // 添加透传类型标识
    cJSON_AddStringToObject(params, "type", "transparent");
    
    // Base64编码透传数据
    size_t encoded_len = 0;
    char* encoded_data = base64_encode((const unsigned char*)data, data_len, &encoded_len);
    if (encoded_data != NULL) {
        cJSON_AddStringToObject(params, "data", encoded_data);
        free(encoded_data);
    } else {
        log_warn("Base64 encoding failed");
        cJSON_Delete(root);
        return -1;
    }
    
    // 添加时间戳
    cJSON_AddNumberToObject(params, "timestamp", HAL_GetTick());
    
    // 转换为JSON字符串并发送
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str != NULL) {
        udp_send_remote((uint8_t *)json_str, strlen(json_str));
        free(json_str);
    }
    
    cJSON_Delete(root);
    
    log_info("Transparent data sent to host, length: %d", data_len);
    return 0;
}

/*******************************************************
* @brief XML待机上报开关设置RPC函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_xml_standby_report(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(params, "enable");
    
    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    // set分支
    if (0 == strcmp(mode->valuestring, "set"))
    {
        if (!cJSON_IsBool(state))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();
        
        // 设置XML待机上报开关状态
        rt_param.xml_standby_report_en = cJSON_IsTrue(state) ? 1 : 0;
        
        // 保存参数到Flash
        save_rt_param_to_flash();
        
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    // get分支
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        
        // 获取当前XML待机上报开关状态
        cJSON_AddBoolToObject(result, "enable", (rt_param.xml_standby_report_en == 1) ? true : false);
        
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 参数解析错误，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/*******************************************************
* @brief 异常关机信息查询RPC函数
*
* @param params 请求参数
* @param id     JSON-RPC调用ID
* @param resJson 输出的结果对象
* @return int 0 表示成功，负值表示失败
*******************************************************/
int rpc_info_error_poweroff(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    if (0 == strcmp(mode->valuestring, "get"))
    {
        int power_on = power_monitor_get_power_on_count();
        int power_off = power_monitor_get_power_off_count();
        int abnormal_count = power_on - power_off;
        
        cJSON *result = cJSON_CreateObject();
        cJSON_AddNumberToObject(result, "power_on", (double)power_on);
        cJSON_AddNumberToObject(result, "power_off", (double)power_off);
        cJSON_AddNumberToObject(result, "error_poweroff", (double)abnormal_count);

        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if (0 == strcmp(mode->valuestring, "set"))
    {
        power_monitor_reset_counters();

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }

    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
    return -1;
}


/*******************************************************
* @brief 硬件版本获取RPC函数
*
* @param params 请求参数
* @param id     JSON-RPC调用ID
* @param resJson 输出的结果对象
* @return int 0 表示成功，负值表示失败
*******************************************************/
int rpc_info_hw_version(cJSON *params, cJSON *id, cJSON **resJson)
{
    if(factoryModel < 2) // 工厂模式小于1，则不支持设置修改和查询
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }
    
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");

    // 参数检查
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    // get分支
    if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        
        // 获取硬件版本号
        uint8_t hw_ver = read_hw_version();
        
        cJSON_AddNumberToObject(result, "hw_version", hw_ver);
        
        // 返回结果
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        // 不支持其它指令，定义为非法参数
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}


/*******************************************************
* @brief 网络配置设置RPC函数
* 
* @param params 
* @param id 
* @param resJson 
* @return int 
*******************************************************/
int rpc_com_ip(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    if (0 == strcmp(mode->valuestring, "set"))
    {
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(params, "ip");
        cJSON *mask = cJSON_GetObjectItemCaseSensitive(params, "mask");
        cJSON *gateway = cJSON_GetObjectItemCaseSensitive(params, "gw");
        
        char ip_str[20] = {0};
        char mask_str[20] = {0};
        char gateway_str[20] = {0};

        if(ip != NULL)
        {
            if(cJSON_IsString(ip) && isVaildIp(ip->valuestring))
            {
                strcpy(ip_str, ip->valuestring);
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }
        if(mask != NULL )
        {
            if(cJSON_IsString(mask) && isVaildIp(mask->valuestring))
            {
                strcpy(mask_str, mask->valuestring);
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }
        if(gateway != NULL )
        {
            if(cJSON_IsString(gateway) && isVaildIp(gateway->valuestring))
            {
                strcpy(gateway_str, gateway->valuestring);
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        
        // 设置IP，并记录
        set_ip_config(ip_str, mask_str, gateway_str);
        // 同步给Mstar
        uint8_t  payload[16] = {0};
        payload[0] = PARAM_ID_LOCAL_IP_SET; 
        memcpy(payload + 1, rt_param.ip_addr, 4);
        memcpy(payload + 5, rt_param.mask, 4);
        memcpy(payload + 9, rt_param.gateway, 4);   
        appSendUartAsync(MST_CMD_PARAM_SET, payload, 13);
        return 0;

    }
    else if (0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        
        char ip_str[20] = {0};
        char mask_str[20] = {0};
        char gateway_str[20] = {0};
        char mac_str[20] = {0};
        get_ip_config(ip_str, mask_str, gateway_str);
        sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
            lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
        cJSON_AddStringToObject(result, "ip", ip_str);
        cJSON_AddStringToObject(result, "mask", mask_str);
        cJSON_AddStringToObject(result, "gateway", gateway_str);
        cJSON_AddStringToObject(result, "mac", mac_str);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}


int rpc_report_confg(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    if(0 == strcmp(mode->valuestring, "set"))
    {
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(params, "ip");
        cJSON *port = cJSON_GetObjectItemCaseSensitive(params, "port");
        if(ip != NULL)
        {
            if(cJSON_IsString(ip) && cJSON_IsNumber(port) && isVaildIp(ip->valuestring))
            {
                set_report_ip(ip->valuestring, port->valueint);
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }

        cJSON *en = cJSON_GetObjectItemCaseSensitive(params, "enable");
        if(en != NULL)
        {
            if(cJSON_IsBool(en))
            {
                rt_param.health_report_en = cJSON_IsTrue(en) ? 1 : 0;
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }

        cJSON *format = cJSON_GetObjectItemCaseSensitive(params, "format");
        if(format != NULL)
        {
            if(cJSON_IsString(format))
            {
                if(0 == strcmp(format->valuestring, "xml"))
                {
                    rt_param.health_report_format = 1;
                }
                else if(0 == strcmp(format->valuestring, "byte"))
                {
                    rt_param.health_report_format = 0;
                }
                else
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                    return -1;
                }
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }

        cJSON *peroid = cJSON_GetObjectItemCaseSensitive(params, "interval");
        if(peroid != NULL)
        {
            if(cJSON_IsNumber(peroid))
            {
                rt_param.health_report_period = peroid->valueint;
            }
            else
            {
                constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
                return -1;
            }
        }

        save_rt_param_to_flash();

        // 同步给Mstar
        uint8_t syncData[12] = {0};
        syncData[0] = PARAM_ID_REPORT_IP_SET;
        syncData[1] = rt_param.report_ip[0];
        syncData[2] = rt_param.report_ip[1];
        syncData[3] = rt_param.report_ip[2];
        syncData[4] = rt_param.report_ip[3];
        syncData[5] = rt_param.report_port >> 8 & 0xFF;
        syncData[6] = rt_param.report_port & 0xFF;
        syncData[7] = rt_param.health_report_en ? 1 : 0;
        syncData[8] = rt_param.health_report_format;
        syncData[9] = rt_param.health_report_period >> 8 & 0xFF;
        syncData[10] = rt_param.health_report_period & 0xFF;        
        appSendUartAsync(MST_CMD_PARAM_SET, (uint8_t*)syncData, 11);

        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if(0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        char ip_str[20] = {0};
        int port = 0;
        get_report_ip(ip_str, &port);
        cJSON_AddStringToObject(result, "ip", ip_str);
        cJSON_AddNumberToObject(result, "port", port);
        // 增加上报开关、格式、间隔参数
        cJSON_AddBoolToObject(result, "enable", rt_param.health_report_en);
        cJSON_AddStringToObject(result, "format", rt_param.health_report_format == 1 ? "xml" : "byte");
        cJSON_AddNumberToObject(result, "interval", rt_param.health_report_period);

        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else 
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}



/**
 * @brief 运行时参数设置、获取指令接口
 * @param params 
 * @param id 
 * @param resJson 
 * @return 
 */
int rpc_rt_param(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if (!cJSON_IsString(mode))
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_QUEST, 0, resJson);
        return -1;
    }

    if(0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        cJSON *keys = cJSON_GetObjectItemCaseSensitive(params, "keys");

        //解析字符串数组
        if(cJSON_IsArray(keys) && keys->child != NULL)
        {
            cJSON *item = NULL;
            cJSON_ArrayForEach(item, keys)
            {
                if(cJSON_IsString(item))
                {
                    if(0 == strcmp(item->valuestring, "display_mode"))
                    {
                        const char* desc = get_param_desc_str(rt_param.display_mode, dispModeStrTab);
                        cJSON_AddStringToObject(result, "display_mode", desc);
                    }
                    else if(0 == strcmp(item->valuestring, "color_space"))
                    {
                        const char* desc = get_param_desc_str(rt_param.color_space, colorSpaceStrTable);
                        cJSON_AddStringToObject(result, "color_space", desc);
                    }
                    else if(0 == strcmp(item->valuestring, "color_temp"))
                    {
                        const char* desc = get_param_desc_str(rt_param.color_temp, colorTempStrTable);
                        cJSON_AddStringToObject(result, "color_temp", desc);
                    }
                    else if(0 == strcmp(item->valuestring, "brightness_level"))
                    {
                        cJSON_AddNumberToObject(result, "brightness_level", rt_param.laser_lvl);
                    }
                    else if(0 == strcmp(item->valuestring, "laser_en"))
                    {
                        cJSON_AddBoolToObject(result, "laser_en", rt_param.laser_en);
                    }
                    else if (0 == strcmp(item->valuestring, "ir_en"))
                    {
                        cJSON_AddBoolToObject(result, "ir_en", rt_param.ir_en);
                    }
                    else if (0 == strcmp(item->valuestring, "ir_lvl"))
                    {
                        cJSON_AddNumberToObject(result, "ir_lvl", rt_param.ir_lvl);
                    }
                    else if (0 == strcmp(item->valuestring, "ir_id"))
                    {
                        cJSON_AddNumberToObject(result, "ir_id", rt_param.ir_id);
                    }
                    else if (0 == strcmp(item->valuestring, "installation_mode"))
                    {
                        const char* desc = get_param_desc_str(rt_param.installation_mode, installationModeStrTable);
                        cJSON_AddStringToObject(result, "installation_mode", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "high_altitude"))
                    {
                        cJSON_AddBoolToObject(result, "high_altitude", rt_param.high_altitude);
                    }
                    else if (0 == strcmp(item->valuestring, "eco"))
                    {
                        cJSON_AddBoolToObject(result, "eco", rt_param.eco_mode_en);
                    }
                    else if (0 == strcmp(item->valuestring, "input_source"))
                    {
                        const char* desc = get_param_desc_str(rt_param.input_source, inputSourceStrTable);
                        cJSON_AddStringToObject(result, "input_source", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "input_color_format"))
                    {
                        const char* desc = get_param_desc_str(rt_param.input_color_format, inputColorFormatStrTable);
                        cJSON_AddStringToObject(result, "input_color_format", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "input_color_space"))
                    {
                        const char* desc = get_param_desc_str(rt_param.input_color_space, inputColorSpaceStrTable);
                        cJSON_AddStringToObject(result, "input_color_space", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "contrast"))
                    {
                        cJSON_AddNumberToObject(result, "contrast", rt_param.contrast);
                    }
                    else if (0 == strcmp(item->valuestring, "sharpness"))
                    {
                        cJSON_AddNumberToObject(result, "sharpness", rt_param.sharpness);
                    }
                    else if (0 == strcmp(item->valuestring, "brightness"))
                    {
                        cJSON_AddNumberToObject(result, "brightness", rt_param.brightness);
                    }
                    else if (0 == strcmp(item->valuestring, "gamma"))
                    {
                        const char* desc = get_param_desc_str(rt_param.gamma, gammaStrTable);
                        cJSON_AddStringToObject(result, "gamma", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "background_color"))
                    {
                        const char* desc = get_param_desc_str(rt_param.background_color, backgroundColorStrTable);
                        cJSON_AddStringToObject(result, "background_color", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "lens_lock"))
                    {
                        cJSON_AddBoolToObject(result, "lens_lock", rt_param.lens_lock);
                    }
                    else if (0 == strcmp(item->valuestring, "alarm_en"))
                    {
                        cJSON_AddBoolToObject(result, "alarm_en", rt_param.alarm_en);
                    }
                    else if (0 == strcmp(item->valuestring, "alarm_tip"))
                    {
                        cJSON_AddBoolToObject(result, "alarm_tip", rt_param.alarm_tip);
                    }
                    else if (0 == strcmp(item->valuestring, "no_input_poweroff"))
                    {
                        cJSON_AddBoolToObject(result, "no_input_poweroff", rt_param.no_input_poweroff);
                    }
                    else if (0 == strcmp(item->valuestring, "aspect_ratio"))
                    {
                        const char* desc = get_param_desc_str(rt_param.aspect_ratio, aspectRatioStrTable);
                        cJSON_AddStringToObject(result, "aspect_ratio", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "language"))
                    {
                        const char* desc = get_param_desc_str(rt_param.language, languageStrTable);
                        cJSON_AddStringToObject(result, "language", desc);
                    }
                    else if (0 == strcmp(item->valuestring, "hsg_param"))
                    {
                        cJSON *hsg_param = cJSON_CreateArray();
                        for (int i = 0; i < 21; i++)
                            cJSON_AddItemToArray(hsg_param, cJSON_CreateNumber(rt_param.HSG_param[i]));
                        cJSON_AddItemToObject(result, "hsg_param", hsg_param);                        
                    }
                    else if (0 == strcmp(item->valuestring, "rgb_gain"))
                    {
                        cJSON *rgb_gain = cJSON_CreateArray();
                        for (int i = 0; i < 3; i++)
                            cJSON_AddItemToArray(rgb_gain, cJSON_CreateNumber(rt_param.RGB_gain[i]));
                        cJSON_AddItemToObject(result, "rgb_gain", rgb_gain);
                    }
                    else if (0 == strcmp(item->valuestring, "indep_color"))
                    {
                        cJSON *indep_color = cJSON_CreateArray();
                        for (int i = 0; i < 6; i++)
                            cJSON_AddItemToArray(indep_color, cJSON_CreateNumber(rt_param.indep_color[i]));
                        cJSON_AddItemToObject(result, "indep_color", indep_color);
                    }
                    else if (0 == strcmp(item->valuestring, "power_auto_boot"))
                    {
                        cJSON_AddBoolToObject(result, "power_auto_boot", rt_param.power_auto_boot);
                    }
                    else if (0 == strcmp(item->valuestring, "model"))
                    {
                        cJSON_AddStringToObject(result, "model", rt_param.display_model);
                    }
                    else if (0 == strcmp(item->valuestring, "serial"))
                    {
                        cJSON_AddStringToObject(result, "serial", rt_param.display_sn);
                    }
                    else if (0 == strcmp(item->valuestring, "date"))
                    {
                        cJSON_AddStringToObject(result, "date", rt_param.display_date);
                    }
                }
            }
        }
        else
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, 0, resJson);
            return -1;
        }

        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else if(0 == strcmp(mode->valuestring, "set"))
    {
        cJSON *kv_pairs = cJSON_GetObjectItemCaseSensitive(params, "kvs");
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, kv_pairs)
        {
            static uint8_t payload[64] = {0};

            if(0 == strcmp(item->string, "disp_mode"))
            {
                int mode_id = get_param_id_by_value(item->valuestring, dispModeStrTab);
                if(mode_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.display_mode = mode_id;
                payload[0] = PARAM_ID_DISPLAY_MODE;
                payload[1] = rt_param.display_mode;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "color_space"))
            {
                int color_space_id = get_param_id_by_value(item->valuestring, colorSpaceStrTable);
                if(color_space_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.color_space = color_space_id;
                payload[0] = PARAM_ID_COLOR_SPACE;
                payload[1] = rt_param.color_space;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "color_temp"))
            {
                int color_temp_id = get_param_id_by_value(item->valuestring, colorTempStrTable);
                if(color_temp_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.color_temp = color_temp_id;
                payload[0] = PARAM_ID_COLOR_TEMP;
                payload[1] = rt_param.color_temp;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "brightness_level"))
            {
                rt_param.laser_lvl = item->valueint;
                payload[0] = PARAM_ID_LASER_BRIGHT;
                payload[1] = rt_param.laser_lvl;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "laser_en"))
            {
                rt_param.laser_en = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_LASER_EN;
                payload[1] = rt_param.laser_en;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "ir_en"))
            {
                rt_param.ir_en = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_IR_EN;
                payload[1] = rt_param.ir_en;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "ir_lvl"))
            {
                rt_param.ir_lvl = item->valueint;
                payload[0] = PARAM_ID_IR_BRIGHT;
                payload[1] = rt_param.ir_lvl;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "ir_id"))
            {
                rt_param.ir_id = item->valueint;
                payload[0] = PARAM_ID_IR_ID;
                payload[1] = rt_param.ir_id;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "installation_mode"))
            {
                int installation_mode_id = get_param_id_by_value(item->valuestring, installationModeStrTable);
                if(installation_mode_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.installation_mode = installation_mode_id;
                payload[0] = PARAM_ID_INSTALLATION_MODE;
                payload[1] = rt_param.installation_mode;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "high_altitude"))
            {
                rt_param.high_altitude = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_HIGH_ALTITUDE;
                payload[1] = rt_param.high_altitude;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
                modectl_switch_mode(rt_param.high_altitude == 1 ? MODECTL_CMD_HI_ALT_ON : MODECTL_CMD_HI_ALT_OFF);
            }
            else if(0 == strcmp(item->string, "eco"))
            {
                rt_param.eco_mode_en = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_ECO_MODE_EN;
                payload[1] = rt_param.eco_mode_en;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
                modectl_switch_mode(rt_param.eco_mode_en == 1 ? MODECTL_CMD_ECO_ON : MODECTL_CMD_ECO_OFF);
            }
            else if(0 == strcmp(item->string, "input_source"))
            {
                int input_source_id = get_param_id_by_value(item->valuestring, inputSourceStrTable);
                if(input_source_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.input_source = input_source_id;
                rt_param.video_in_chan = input_source_id;

                payload[0] = PARAM_ID_INPUT_SOURCE;
                payload[1] = rt_param.input_source;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
                
            #if (Projector_Model == LP92SLB)
                // 切换HDMI输入端口
                if(input_source_id == VIDEO_INPUT_SRC_HDMI_1 || input_source_id == VIDEO_INPUT_SRC_HDMI_2)
                {
                    video_src_changed = TRUE;
                }
            #endif
            }
            else if(0 == strcmp(item->string, "input_color_format"))
            {
                int input_color_format_id = get_param_id_by_value(item->valuestring, inputColorFormatStrTable);
                if(input_color_format_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.input_color_format = input_color_format_id;
                payload[0] = PARAM_ID_INPUT_COLOR_FORMAT;
                payload[1] = rt_param.input_color_format;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "input_color_space"))
            {
                int input_color_space_id = get_param_id_by_value(item->valuestring, inputColorSpaceStrTable);
                if(input_color_space_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.input_color_space = input_color_space_id;
                payload[0] = PARAM_ID_INPUT_COLOR_SPACE;
                payload[1] = rt_param.input_color_space;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "contrast"))
            {
                rt_param.contrast = item->valueint;
                payload[0] = PARAM_ID_CONTRAST;
                payload[1] = rt_param.contrast;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "sharpness"))
            {
                rt_param.sharpness = item->valueint;
                payload[0] = PARAM_ID_SHARPNESS;
                payload[1] = rt_param.sharpness;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "brightness"))
            {
                rt_param.brightness = item->valueint;
                payload[0] = PARAM_ID_BRIGHTNESS;
                payload[1] = rt_param.brightness;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "gamma"))
            {
                int gamma_id = get_param_id_by_value(item->valuestring, gammaStrTable);
                if(gamma_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.gamma = gamma_id;
                payload[0] = PARAM_ID_GAMMA;
                payload[1] = rt_param.gamma;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "background_color"))
            {
                int color_id = get_param_id_by_value(item->valuestring, backgroundColorStrTable);   
                if(color_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.background_color = color_id;
                payload[0] = PARAM_ID_BACKGROUND_COLOR;
                payload[1] = rt_param.background_color;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "lens_lock"))
            {
                rt_param.lens_lock = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_LENS_LOCK;
                payload[1] = rt_param.lens_lock;
                OE_lock = (rt_param.lens_lock == 1) ? TRUE : FALSE;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "alarm_en"))
            {
                rt_param.alarm_en = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_ALARM_EN;
                payload[1] = rt_param.alarm_en;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "alarm_tip"))
            {
                rt_param.alarm_tip = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_ALARM_TIP;
                payload[1] = rt_param.alarm_tip;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "no_input_poweroff"))
            {
                rt_param.no_input_poweroff = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_NO_INPUT_POWEROFF;
                payload[1] = rt_param.no_input_poweroff;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "aspect_ratio"))
            {
                int aspect_ratio_id = get_param_id_by_value(item->valuestring, aspectRatioStrTable);
                if(aspect_ratio_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.aspect_ratio = aspect_ratio_id;
                payload[0] = PARAM_ID_ASPECT_RATIO;
                payload[1] = rt_param.aspect_ratio;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "language"))
            {
                int language_id = get_param_id_by_value(item->valuestring, languageStrTable);
                if(language_id < 0)
                {
                    constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
                    return -1;
                }
                rt_param.language = language_id;
                payload[0] = PARAM_ID_LANGUAGE;
                payload[1] = rt_param.language;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            else if(0 == strcmp(item->string, "hsg_param"))
            {
                cJSON *sub_item = NULL;
                int i = 0;
                cJSON_ArrayForEach(sub_item, item)
                {
                    if(i < 21)
                    {
                        rt_param.HSG_param[i] = sub_item->valueint;
                        i++;
                    }
                }
                payload[0] = PARAM_ID_HSG_PRARM;
                for(int j = 0; j < 21; j++)
                {
                    payload[j + 1] = rt_param.HSG_param[j];
                }
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 22);
            }
            else if(0 == strcmp(item->string, "rgb_gain"))
            {
                cJSON *sub_item = NULL;
                int i = 0;
                cJSON_ArrayForEach(sub_item, item)
                {
                    if(i < 3)
                    {
                        rt_param.RGB_gain[i] = sub_item->valueint;
                        i++;
                    }
                }
                payload[0] = PARAM_ID_RGB_GAIN;
                for(int j = 0; j < 3; j++)
                {
                    payload[j + 1] = rt_param.RGB_gain[j];
                }
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 4);
            }
            else if(0 == strcmp(item->string, "indep_color"))
            {
                cJSON *sub_item = NULL;
                int i = 0;
                cJSON_ArrayForEach(sub_item, item)
                {
                    if(i < 6)
                    {
                        rt_param.indep_color[i] = sub_item->valueint;
                        i++;
                    }
                }
                payload[0] = PARAM_ID_INDEP_COLOR;
                for(int j = 0; j < 6; j++)
                {
                    payload[j + 1] = rt_param.indep_color[j];
                }
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 7);
            }
            else if (0 == strcmp(item->string, "power_auto_boot"))
            {
                rt_param.power_auto_boot = cJSON_IsTrue(item) ? 1 : 0;
                payload[0] = PARAM_ID_POWER_AUTO_BOOT;
                payload[1] = rt_param.power_auto_boot;
                appSendUartAsync(MST_CMD_PARAM_SET, payload, 2);
            }
            HAL_Delay(10);//等待10ms 给mstar 处理
        }
        // 保存参数到flash
        save_rt_param_to_flash();
        // 返回结果
        cJSON *result = cJSON_CreateObject();
        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}

/**
 * @brief 工厂模式密码认证
 * @param params 
 * @param id 
 * @param resJson 
 * @return 
 */
int rpc_fty_mode_auth(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if(0 == strcmp(mode->valuestring, "set"))
    {
        cJSON *password = cJSON_GetObjectItemCaseSensitive(params, "password");
        //判断密码是数字格式，并且符合三个level的密码定义，则进入不同的工厂模式
        if(!cJSON_IsNumber(password))
        {
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }

        cJSON *result = cJSON_CreateObject();

        if(password->valueint == FACTORY_MODE_PASSWORD_LVL_1)
        {
            factoryModel = 1;
        }
        else if(password->valueint == FACTORY_MODE_PASSWORD_LVL_2)
        {
            factoryModel = 2;
        }
        else if(password->valueint == FACTORY_MODE_PASSWORD_LVL_3)
        {
            factoryModel = 3;
        }
        else
        {
            cJSON_AddBoolToObject(result, "return", false);
            constructRpc20Result(result, id->valueint, resJson);
            return -1;
        }

        #define MST_CMD_FACTORY_MODE_AUTH 0X33 // 工厂模式密码认证指令
        appSendUartAsync(MST_CMD_FACTORY_MODE_AUTH, &factoryModel, 1);

        cJSON_AddBoolToObject(result, "return", true);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;

    }
    else if( 0 == strcmp(mode->valuestring, "get"))
    {
        cJSON *result = cJSON_CreateObject();
        cJSON_AddNumberToObject(result, "factory_mode_level", factoryModel);
        constructRpc20Result(result, id->valueint, resJson);
        return 0;
    }
    else
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
}


int rpc_quick_ops(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if(0 == strcmp(mode->valuestring, "set"))
    {
        cJSON *ops = cJSON_GetObjectItemCaseSensitive(params, "ops");
        uint8_t payload[1];
        
        if (0 == strcmp(ops->valuestring, "0"))
        {
            payload[0] = 0x00;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "1"))
        {
            payload[0] = 0x01;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "2"))
        {
            payload[0] = 0x02;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "3"))
        {
            payload[0] = 0x03;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "4"))
        {
            payload[0] = 0x04;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "5"))
        {
            payload[0] = 0x05;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "6"))
        {
            payload[0] = 0x06;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "7"))
        {
            payload[0] = 0x07;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "8"))
        {
            payload[0] = 0x08;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "9"))
        {
            payload[0] = 0x09;
            appSendUartAsync(0x32, payload, 1);
        }
        else if(0 == strcmp(ops->valuestring, "menu"))
        {
            payload[0] = 0x0A;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "enter"))
        {
            payload[0] = 0x0B;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "exit"))
        {
            payload[0] = 0x0C;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "left"))
        {
            payload[0] = 0x0D;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "right"))
        {
            payload[0] = 0x0E;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "up"))
        {
            payload[0] = 0x0F;
            appSendUartAsync(0x32, payload, 1);
        }
        else if (0 == strcmp(ops->valuestring, "down"))
        {
            payload[0] = 0x10;
            appSendUartAsync(0x32, payload, 1);
        }
        else{
            constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
            return -1;
        }
    }
    else 
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }
    cJSON *result = cJSON_CreateObject();
    cJSON_AddTrueToObject(result, "return");
    constructRpc20Result(result, id->valueint, resJson);
    return 0;
}


int rpc_sys_reset(cJSON *params, cJSON *id, cJSON **resJson)
{
    cJSON *mode = cJSON_GetObjectItemCaseSensitive(params, "mode");
    if(0 == strcmp(mode->valuestring, "set"))
    {
        rt_param_reset();
        rt_param_sync_to_mstar();
    }
    else 
    {
        constructRpc20ErrJson(JSONRPC20_INVALID_PARAM, id->valueint, resJson);
        return -1;
    }

    cJSON *result = cJSON_CreateObject();
    cJSON_AddBoolToObject(result, "return", true);
    constructRpc20Result(result, id->valueint, resJson);
    return 0;
}
