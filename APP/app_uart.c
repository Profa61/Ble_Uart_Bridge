/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : zhangxiyi @WCH
 * Version            : v0.1
 * Date               : 2020/11/26
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "app_uart.h"

/*********************************************************************
 * MACROS
 */
//The buffer length should be a power of 2
#define APP_UART_TX_BUFFER_LENGTH    512U
#define APP_UART_RX_BUFFER_LENGTH    2048U
extern volatile uint8_t app_sleep_lock;
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t to_test_buffer[BLE_BUFF_MAX_LEN - 4 - 3];

app_drv_fifo_t app_uart_tx_fifo;
app_drv_fifo_t app_uart_rx_fifo;

//interupt uart rx flag ,clear at main loop
bool uart_rx_flag = false;

//for interrupt rx blcak hole ,when uart rx fifo full
uint8_t for_uart_rx_black_hole = 0;

//fifo length less that MTU-3, retry times
uint32_t uart_to_ble_send_evt_cnt = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES

 */
 extern uint8_t Peripheral_TaskID;
//

//The tx buffer and rx buffer for app_drv_fifo
//length should be a power of 2
static uint8_t app_uart_tx_buffer[APP_UART_TX_BUFFER_LENGTH] = {0};
static uint8_t app_uart_rx_buffer[APP_UART_RX_BUFFER_LENGTH] = {0};


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PROFILE CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      app_uart_process
 *
 * @brief   process uart data
 *
 * @return  NULL
 */

void app_uart_process(void)
{
    UINT32 irq_status;
    SYS_DisableAllIrq(&irq_status);
    
    if (uart_rx_flag)
    {
        // §µ§¢§²§¡§­§ª §°§´§³§À§¥§¡ tmos_start_task!
        // §´§Ö§á§Ö§â§î §Ù§Õ§Ö§ã§î §ä§à§Ý§î§Ü§à §ã§Ò§â§à§ã §æ§Ý§Ñ§Ô§Ñ, §Ñ §à§ä§á§â§Ñ§Ó§Ü§Ñ §å§á§â§Ñ§Ó§Ý§ñ§Ö§ä§ã§ñ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Ö§Þ §á§à §ä§Ñ§Û§Þ§Ñ§å§ä§å §Ý§Ú§ß§Ú§Ú
        uart_rx_flag = false;
    }
    SYS_RecoverIrq(irq_status);

    //tx process ¡ª §ª§³§±§²§¡§£§­§¦§¯§°: §á§Ö§â§Ö§Ó§Ö§Ý§Ú §à§ä§á§â§Ñ§Ó§Ü§å §Ú§Ù BLE §ß§Ñ §æ§Ú§Ù§Ú§é§Ö§ã§Ü§Ú§Û UART1 (§â§Ñ§Ù§ì§Ö§Þ XP1)
    if (R8_UART1_TFC < UART_FIFO_SIZE)
    {
        app_drv_fifo_read_to_same_addr(&app_uart_tx_fifo, (uint8_t *)&R8_UART1_THR, UART_FIFO_SIZE - R8_UART1_TFC);
    }
}


// void app_uart_process(void)
// {
//     UINT32 irq_status;
//     SYS_DisableAllIrq(&irq_status);
//     if(uart_rx_flag)
//     {
//         tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 12);
//         uart_rx_flag = false;
//     }
//     SYS_RecoverIrq(irq_status);

//     //tx process
//     if(R8_UART1_TFC < UART_FIFO_SIZE)
//     {
//         app_drv_fifo_read_to_same_addr(&app_uart_tx_fifo, (uint8_t *)&R8_UART1_THR, UART_FIFO_SIZE - R8_UART1_TFC);
//     }
// }

/*********************************************************************
 * @fn      app_uart_init
 *
 * @brief   init uart
 *
 * @return  NULL
 */
void app_uart_init()
{
    //tx fifo and tx fifo
    //The buffer length should be a power of 2
    app_drv_fifo_init(&app_uart_tx_fifo, app_uart_tx_buffer, APP_UART_TX_BUFFER_LENGTH);
    app_drv_fifo_init(&app_uart_rx_fifo, app_uart_rx_buffer, APP_UART_RX_BUFFER_LENGTH);

    //uart tx io
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);

    //uart rx io
    GPIOA_SetBits(bRXD1);
    GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);

    //uart3 init
    //uart1 init
    UART1_DefInit();

    //enable interrupt
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART1_IRQn);

}

/*********************************************************************
 * @fn      app_uart_tx_data
 *
 * @brief   app_uart_tx_data
 *
 * @return  NULL
 */
void app_uart_tx_data(uint8_t *data, uint16_t length)
{
    uint16_t write_length = length;
    app_drv_fifo_write(&app_uart_tx_fifo, data, &write_length);
}

/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   Not every uart reception will end with a UART_II_RECV_TOUT
 *          UART_II_RECV_TOUT can only be triggered when R8_UARTx_RFC is not 0
 *          Here we cannot rely UART_II_RECV_TOUT as the end of a uart reception
 *
 * @return  NULL
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    uint16_t error;
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT:
            UART1_GetLinSTA();
            break;

        case UART_II_RECV_RDY:
        case UART_II_RECV_TOUT: {
            uint16_t data_lenght_temp = R8_UART1_RFC;
     if (data_lenght_temp == 0) {
        break; // §©§Ñ§ë§Ú§ä§Ñ §à§ä §á§å§ã§ä§í§ç §Ý§à§Ø§ß§í§ç §ã§â§Ñ§Ò§Ñ§ä§í§Ó§Ñ§ß§Ú§Û §Ý§Ú§ß§Ú§Ú
    }
    
    app_sleep_lock = 1;
            tmos_stop_task(Peripheral_TaskID, UART_GO_TO_SLEEP_EVT);

            // §¹§Ú§ä§Ñ§Ö§Þ §Ú§Ù R8_UART1_RBR §Ü§à§Ý§Ú§é§Ö§ã§ä§Ó§à §Ò§Ñ§Û§ä, §â§Ñ§Ó§ß§à§Ö data_lenght_temp
            error = app_drv_fifo_write_from_same_addr(&app_uart_rx_fifo, (uint8_t *)&R8_UART1_RBR, data_lenght_temp);
            
            if(error != APP_DRV_FIFO_RESULT_SUCCESS)
            {
                // §¦§ã§Ý§Ú §Ò§å§æ§Ö§â §á§Ö§â§Ö§á§à§Ý§ß§Ö§ß, §à§é§Ú§ë§Ñ§Ö§Þ FIFO §ã §æ§Ú§Ü§ã§Ú§â§à§Ó§Ñ§ß§ß§í§Þ §ã§é§Ö§ä§é§Ú§Ü§à§Þ
                for (uint8_t i = 0; i < data_lenght_temp; i++)
                {
                    for_uart_rx_black_hole = R8_UART1_RBR;
                }
            }
            
            uart_rx_flag = true;
           // PRINT("GET UART");
            // §±§²§ª§¯§µ§¥§ª§´§¦§­§¾§¯§° §©§¡§±§µ§³§¬§¡§¦§® §´§¡§³§¬ §°§´§±§²§¡§£§¬§ª §£ BLE §±§²§Á§®§° §³§¦§«§¹§¡§³:
            //tmos_set_event(Peripheral_TaskID, UART_TO_BLE_SEND_EVT);
            tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 8);
            break;

        }
        case UART_II_THR_EMPTY:
            break;
        case UART_II_MODEM_CHG:
            break;
        default:
            break;
    }
}


/*********************************************************************
 * @fn      on_bleuartServiceEvt
 *
 * @brief   ble uart service callback handler
 *
 * @return  NULL
 */
void on_bleuartServiceEvt(uint16_t connection_handle, ble_uart_evt_t *p_evt)
{
    switch(p_evt->type)
    {
        case BLE_UART_EVT_TX_NOTI_DISABLED:
            //PRINT("%02x:bleuart_EVT_TX_NOTI_DISABLED\r\n", connection_handle);
            break;
        case BLE_UART_EVT_TX_NOTI_ENABLED:
            //PRINT("%02x:bleuart_EVT_TX_NOTI_ENABLED\r\n", connection_handle);
            break;
        case BLE_UART_EVT_BLE_DATA_RECIEVED:
            // PRINT("BLE RX DATA len:%d\r\n", p_evt->data.length);


// PRINT("BLE RX DATA len:%d, data: ", p_evt->data.length);
// for(int i = 0; i < p_evt->data.length; i++) {
//     PRINT("%c", p_evt->data.p_data[i]);
// }
// PRINT(" [HEX: ");
// for(int i = 0; i < p_evt->data.length; i++) {
//     PRINT("%02X ", p_evt->data.p_data[i]);
// }
// PRINT("]\r\n");


            //for notify back test
            //to ble
            // uint16_t to_write_length = p_evt->data.length;
            // app_drv_fifo_write(&app_uart_rx_fifo, (uint8_t *)p_evt->data.p_data, &to_write_length);
            // tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
            //end of nofify back test

            //ble to uart
            //app_uart_tx_data((uint8_t *)p_evt->data.p_data, p_evt->data.length);
                    for(int i = 0; i < p_evt->data.length; i++) 
        {
            UART1_SendByte(p_evt->data.p_data[i]);
        }
            break;
        default:
            break;
    }
}

/*********************************************************************
*********************************************************************/
