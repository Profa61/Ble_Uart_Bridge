
#include"pch.h"
#include "app_drv_fifo.h"

extern app_drv_fifo_t app_uart_rx_fifo; 
extern app_drv_fifo_t app_uart_tx_fifo;
extern uint8_t Peripheral_TaskID;  // Импортируем ID задачи из peripheral.c
#define APP_UART_TX_EVT   0x0002

void answering_machine(uint8 *pValue, uint16 len)
{

    // Оставляем только уникальные заголовки команд для быстрой проверки (по 4 байта)
    static const uint8 header1[] = {0x04, 0x02, 0x03, 0xEF};  //1007
    static const uint8 header2[] = {0x17, 0x02, 0x03, 0xF1};   //1009
    static const uint8 header3[] = {0x04, 0x02, 0x03, 0xFA};  // 1018
    static const uint8 header4[] = {0x04, 0x02, 0x03, 0xFB};  // 1019
    static const uint8 header5[] = {0x07, 0x02, 0x03, 0xF0};  //1008
    static const uint8 header6[] = {0x07, 0x01, 0x03, 0xF0};  //1008
    static const uint8 header7[] = {0x09, 0x02, 0x04, 0x85};  //1157
    // Полные массивы ответов
    static const uint8 cmd1_rsp[] = {0x04, 0x03, 0x03, 0xEF, 0x01, 0x4A, 0x2B}; //1007
    static const uint8 cmd2_rsp[] = {0x17, 0x03, 0x03, 0xF1, 0x56, 0x45, 0x47, 0x41, 0x20, 0x53, 0x49, 0x31, 0x31, 0x20, 0x32, 0x2E, 0x37, 0x45, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x34};//1009
    static const uint8 cmd3_rsp[] = {0x04, 0x03, 0x03, 0xFA, 0x18, 0x35, 0xB5}; // 1018
    static const uint8 cmd4_rsp[] = {0x04, 0x03, 0x03, 0xFB, 0x53, 0xFF, 0x2B};  // 1019
    static const uint8 cmd5_rsp[] = {0x07, 0x03, 0x03, 0xF0, 0x6A, 0x7A, 0x93, 0x37, 0xFA, 0xAE}; //1008
    static const uint8 cmd6_rsp[] = {0x07, 0x03, 0x03, 0xF0, 0x6A, 0x7A, 0x93, 0x37, 0xFA, 0xAE}; //1008
    static const uint8 cmd7_rsp[] = {0x09, 0x03, 0x04, 0x85, 0xDE, 0xB0, 0x74, 0x90, 0x73, 0x7E, 0x0C, 0x87};  //1157  
    const uint8 *rsp_ptr = NULL;
    uint16_t rsp_len = 0;

    // Сверяем первые 4 байта пришедшего пакета
    if (len >= 4 && (memcmp(pValue, header1, 4) == 0))
    {
        rsp_ptr = cmd1_rsp;
        rsp_len = sizeof(cmd1_rsp);
    }
    else if (len >= 4 && (memcmp(pValue, header2, 4) == 0))
    {
        rsp_ptr = cmd2_rsp;
        rsp_len = sizeof(cmd2_rsp);
    }
    else if (len >= 4 && (memcmp(pValue, header3, 4) == 0))
    {
        rsp_ptr = cmd3_rsp;
        rsp_len = sizeof(cmd3_rsp);
    }
    else if (len >= 4 && (memcmp(pValue, header4, 4) == 0))
    {
        rsp_ptr = cmd4_rsp;
        rsp_len = sizeof(cmd4_rsp);
    }    
    else if (len >= 4 && (memcmp(pValue, header5, 4) == 0))
    {
        rsp_ptr = cmd5_rsp;
        rsp_len = sizeof(cmd5_rsp);
    }    
    else if (len >= 4 && (memcmp(pValue, header6, 4) == 0))
    {
        rsp_ptr = cmd6_rsp;
        rsp_len = sizeof(cmd6_rsp);
    }    
    else if (len >= 4 && (memcmp(pValue, header7, 4) == 0))
    {
        rsp_ptr = cmd7_rsp;
        rsp_len = sizeof(cmd7_rsp);
    }        
    // Если заголовок совпал, отправляем соответствующий полный ответ в буфер
    if (rsp_ptr != NULL && rsp_len > 0)
    {
        app_drv_fifo_write(&app_uart_rx_fifo, (uint8 *)rsp_ptr, &rsp_len);
        #define UART_TO_BLE_SEND_EVT 0x0002 // Проверьте этот флаг в peripheral.h / peripheral.c
        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
    }
    
}

void ble_uart_ForwardUartToBle(uint8 *pValue, uint16 len, gattAttribute_t *pAttr)
{
    uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
uint16_t marker_len = 1;
uint16_t write_len = len;
    uint8_t channel_marker = (uuid == 0xEA03) ? 0x03 : 0x05;
    app_drv_fifo_write(&app_uart_tx_fifo, &channel_marker, &marker_len);

    if (write_len > 0)
    {
        app_drv_fifo_write(&app_uart_tx_fifo, pValue, &write_len);
    }
            
            tmos_start_task(Peripheral_TaskID, APP_UART_TX_EVT, 2);

}