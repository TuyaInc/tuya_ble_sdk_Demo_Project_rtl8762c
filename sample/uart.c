/**
*********************************************************************************************************
*               
**********************************************************************************************************
* @file     uart.c
* @brief    uart interrupt demo
* @details
* @author   @gyh
* @date     
* @version  
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include <string.h>
#include "app_task.h"
#include "trace.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_type.h"
#include "tuya_ble_utils.h"
#include "app_section.h"
#include "tuya_ble_port.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"




#define UART_RX_BUFFER_MAX 600

/* Globals ------------------------------------------------------------------*/
static uint8_t UART_RX_Buffer[UART_RX_BUFFER_MAX];
static uint16_t UART_RX_Count = 0;

bool IO_UART_DLPS_Enter_Allowed = false;

/**
  * @brief  Initialize uart global data.
  * @param  No parameter.
  * @return void
  */
void global_data_uart_init(void)
{
    IO_UART_DLPS_Enter_Allowed = false;
    UART_RX_Count = 0;
    memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
}

void uart_dlps_enter_allowed_set(bool flag)
{
    tuya_ble_device_enter_critical();
    IO_UART_DLPS_Enter_Allowed = flag; 
    tuya_ble_device_exit_critical();
       
    APP_PRINT_INFO1("IO_UART_DLPS_Enter_Allowed = %d",IO_UART_DLPS_Enter_Allowed);  
}


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_PullConfigValue(UART_RX_PIN, PAD_STRONG_PULL);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_uart_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    global_data_uart_init();
    /* uart init */
    UART_InitTypeDef UART_InitStruct; 
    UART_StructInit(&UART_InitStruct);

    UART_InitStruct.parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct.stopBits       = UART_STOP_BITS_1;
    UART_InitStruct.wordLen        = UART_WROD_LENGTH_8BIT;
    UART_InitStruct.rxTriggerLevel = 16;                       //1~29
    UART_InitStruct.idle_time      = UART_RX_IDLE_64BYTE;      //idle interrupt wait time
    UART_Init(UART, &UART_InitStruct);

    //enable rx interrupt and line status interrupt
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
    UART_INTConfig(UART, UART_INT_IDLE, ENABLE);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void uart_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, 0);
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void uart_dlps_exit(void)
{
    /* Switch pad to Pinmux mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_PINMUX_MODE);

}

/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
bool uart_dlps_check(void)
{
    return IO_UART_DLPS_Enter_Allowed;
}

/**
  * @brief  UARt send data continuous.
  * @param  No parameter.
  * @return void
  */
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount)
{
    uint8_t count;

    while (vCount / UART_TX_FIFO_SIZE > 0)
    {
        while (UART_GetFlagState(UARTx, UART_FLAG_THR_EMPTY) == 0);
        for (count = UART_TX_FIFO_SIZE; count > 0; count--)
        {
            UARTx->RB_THR = *pSend_Buf++;
        }
        vCount -= UART_TX_FIFO_SIZE;
    }

    while (UART_GetFlagState(UARTx, UART_FLAG_THR_EMPTY) == 0);
    while (vCount--)
    {
        UARTx->RB_THR = *pSend_Buf++;
    }
}


/**
  * @brief  Handle uart data function.
  * @param  No parameter.
  * @return void
  */
/*
void io_uart_handle_msg(T_IO_MSG *io_uart_msg)
{
//    uint8_t *p_buf = io_uart_msg.u.buf;
    uint16_t subtype = io_uart_msg->subtype;

    if (IO_MSG_UART_RX == subtype)
    {
        uart_senddata_continuous(UART, UART_RX_Buffer, UART_RX_Count);
        global_data_uart_init();
        while (UART_GetFlagState(UART, UART_FLAG_THR_EMPTY) == 0) { IO_UART_DLPS_Enter_Allowed = true; }
    }
}

*/
extern void io_uart_dlps_monitor_timer_start(void);

DATA_RAM_FUNCTION static void uart_receive_process(void)
{
   
    //tuya_log_dumpHex("uart_receive_process temp data",20,UART_RX_Buffer,UART_RX_Count);
    
    
    if(UART_RX_Count<7)
    {
        UART_RX_Count = 0;
        memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
        return;
    }
    
    if(tuya_ble_common_uart_receive_data(UART_RX_Buffer,UART_RX_Count)==TUYA_BLE_SUCCESS)
    {
        io_uart_dlps_monitor_timer_start();
    }
  /*  
    for(i=0; i<(UART_RX_Count-2); i++)
    {
        if(((UART_RX_Buffer[i]==0x55)||(UART_RX_Buffer[i]==0x66))&&(UART_RX_Buffer[i+1]==0xAA)&&((UART_RX_Buffer[i+2]==0x01)||(UART_RX_Buffer[i+2]==0x00)))
        {
            if((i+5+1)>UART_RX_Count)
            {
                break;
            }
            else
            {
                data_len = (UART_RX_Buffer[i+4]<<8)|UART_RX_Buffer[i+5];
            }

            if((i+5+data_len+2)>UART_RX_Count)
            {
                continue;
            }
            sum = check_sum(&UART_RX_Buffer[i],data_len+6);
            if(sum==UART_RX_Buffer[i+6+data_len])
            {
                uart_evt_buffer=(uint8_t*)tuya_malloc(data_len+7);
                
                if(uart_evt_buffer==NULL)
                {
                    APP_PRINT_INFO0("tuya_malloc uart evt buffer fail.");
                    break;
                }
                
                uart_event.hdr.event_id = TUYA_UART_EVT;
                uart_event.uart_event.cmd=0;
                uart_event.uart_event.data=uart_evt_buffer;
                uart_event.uart_event.len=data_len+7;
                
                memcpy(uart_evt_buffer,&UART_RX_Buffer[i],data_len+7);
                                
                if(tuya_event_send(&uart_event)!=0)
                {
                   // tuya_MemPut(uart_evt_buffer);
                    tuya_free(uart_evt_buffer);
                    APP_PRINT_INFO0("send uart receive data to task fail.");
                    break;
                }
                APP_PRINT_INFO0("send uart receive data to task success.");              
            }
            else
            {
                APP_PRINT_INFO2("uart receive data check_sum error , receive sum = 0x%02x ; cal sum = 0x%02x",UART_RX_Buffer[i+6+data_len],sum);
            }

        }
    }
    */
    UART_RX_Count = 0;
    memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));

}


DATA_RAM_FUNCTION void UART0_Handler(void)
{
    uint16_t rx_len = 0;

    /* Read interrupt id */
    uint32_t int_status = UART_GetIID(UART);
    //APP_PRINT_INFO1("Read interrupt int_status = 0x%x",int_status); 
    /* Disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);

    if (UART_GetFlagState(UART, UART_FLAG_RX_IDLE) == SET)
    {
        /* Clear flag */
        UART_INTConfig(UART, UART_INT_IDLE, DISABLE);

        uart_receive_process();

        UART_INTConfig(UART, UART_INT_IDLE, ENABLE);
    }

    switch (int_status & 0x0E)
    {
    /* Rx time out(0x0C). */
    case UART_INT_ID_RX_TMEOUT:
        rx_len = UART_GetRxFIFOLen(UART);
        if((UART_RX_Count+rx_len)>UART_RX_BUFFER_MAX)
        {
            UART_RX_Count = 0;
            memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
        }
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Receive line status interrupt(0x06). */
    case UART_INT_ID_LINE_STATUS:
        break;

    /* Rx data valiable(0x04). */
    case UART_INT_ID_RX_LEVEL_REACH:
        rx_len = UART_GetRxFIFOLen(UART);
        if((UART_RX_Count+rx_len)>UART_RX_BUFFER_MAX)
        {
            UART_RX_Count = 0;
            memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
        }
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Tx fifo empty(0x02), not enable. */
    case UART_INT_ID_TX_EMPTY:
        /* Do nothing */
        break;
    default:
        break;
    }

    /* enable interrupt again */
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
