#include "util.h"
#include <stdio.h>

uint8_t simple_checksum(uint8_t* buffer, int len) {  
    uint8_t checksum = 0;  
    for (int i = 0; i < len; i++) {  
        // checksum ^= buffer[i]; // 对每个字节进行异或操作  
        checksum += buffer[i]; // 累加和  
    }  
    return checksum;  
}  

/**
 * @brief 以十六进制格式打印内存数据（带标签）
 * @param tag   指示词/标签名
 * @param buf   数据缓冲区指针
 * @param len   数据长度
 */
void print_mem_hexdump(const char* tag, const void* buf, int len)
{
    const uint8_t* data = (const uint8_t*)buf;
    int i, j;
    
    // 打印标题行
    printf("--> %s (len=%d):\n", tag ? tag : "hexdump", len);
    
    // 按每行16字节打印
    for (i = 0; i < len; i += 16) {
        // 打印地址偏移
        printf("%04X: ", i);
        
        // 打印十六进制数据
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02X ", data[i + j]);
            } else {
                printf("   ");  // 填充空格
            }
            if (j == 7) {
                printf(" ");  // 中间分隔
            }
        }
        
        // 打印ASCII可打印字符
        printf(" |");
        for (j = 0; j < 16 && (i + j) < len; j++) {
            uint8_t c = data[i + j];
            if (c >= 0x20 && c <= 0x7E) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("|\n");
    }
}


/*
* 函数功能：判断字符串是否为合法IP地址
* 函数类型：立即函数，调用后立即生效。
* 函数形参：
*	const char* ip：需要判断的字符串
* 返 回 值：
*	true：IP地址合法正确
*  false：IP地址格式错误
* 其他：
*   1.
*/
bool isVaildIp(const char* ip)
{
    int     dots = 0;                                   //字符 . 的个数
    int     setions = 0;                                //ip每一部分总和（0-255）
    int     strnum = 0;                                 //字符串长度记录

    if (NULL == ip || *ip == '.')                       //排除输入参数为NULL, 或者第一个字符为'.'的字符串    
    {
        return  false;                                  //格式错误
    }

    while (*ip)                                         //字符串不为空
    {
        if (*ip == '.')                                 //如果当前字符为点.则开始判断上一段是否合法在0~255之间
        {
            dots++;                                     //字符 . 的个加一
            if (setions >= 0 && setions <= 255)         //检查ip是否合法
            {
                setions = 0;                            //对上一段IP总和进行清零
            }
            else
            {  
                return  false;                          //格式错误，某一段数值不在0~255之间
            }
        }
        else if (*ip >= '0' && *ip <= '9')              //判断是不是数字
        {
            setions = setions * 10 + (*ip - '0');       //求每一段总和
        }
        else
        {
            return  false;                              //该字符既不是数字也不是点
        }
        ip++;                                           //指向下一个字符
        if (++strnum>15)                                //3*4+3=15,IP地址字符串非结束符字符数最多15个
        {
            return  false;                              //防止出现“1234567890987654321.”这种前面全是数字的字符串输入进来。
        }
    }
    /*判断IP最后一段是否合法*/
    if (setions >= 0 && setions <= 255)
    {                 
        if (dots == 3)                                  //防止出现：“192.168.123”这种不是三个点的情况
        {
            return  true;                               //IP地址合法正确
        }
    }
    return  false;
}
