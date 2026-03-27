#include "uartCmdProc.h"

#include "main.h"
#include "usart.h"
#include <hal/uart_protocal.h>

int uartCmdProcTask(void)
{
#if 0 
    if(g_uart1_ack_flag == 1)
    {
        printf("\r\n --> uart1 rcv ack %d\n", g_uart1_ack_len);
        for (size_t i = 0; i < g_uart1_ack_len; i++)
        {
            printf("%d,",g_uart1_rx_ack_buf[i]);
        }
        printf("\n");
        g_uart1_ack_len = 0 ;
        g_uart1_ack_flag = 0 ;
    }

    if(g_uart1_cmd_flag == 1)
    {
        printf("\r\n --> uart1 rcv cmd %d\n", g_uart1_cmd_len);
        for (size_t i = 0; i < g_uart1_cmd_len; i++)
        {
            printf("%d,",g_uart1_rx_cmd_buf[i]);
        }
        printf("\n");
        g_uart1_cmd_len = 0 ;
        g_uart1_cmd_flag = 0 ;
    }
#else
    if(g_uart1_cmd_flag)
    {
        uartCmdParse();
        g_uart1_cmd_len = 0 ;
        g_uart1_cmd_flag = 0 ;
    }
#endif
    return 0 ;
}

