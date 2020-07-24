
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"
#include "board.h"
#include "app_msg.h"

#define   UART_TX_PIN    P3_0 
#define   UART_RX_PIN    P3_1 

/* Globals ------------------------------------------------------------------*/
extern bool IO_UART_DLPS_Enter_Allowed;

void global_data_uart_init(void);
void board_uart_init(void);
void driver_uart_init(void);
void uart_dlps_enter(void);
void uart_dlps_exit(void);
bool uart_dlps_check(void);
//void io_handle_uart_msg(T_IO_MSG *io_uart_msg);
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount);

void uart_dlps_enter_allowed_set(bool flag);

#ifdef __cplusplus
}
#endif

#endif

