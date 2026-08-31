/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"
#include "peripheral.h"
#include "app_uart.h"
#include "pch.h"
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif
volatile uint8_t app_sleep_lock=0;
/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : 主循环
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        app_uart_process();
        //indicate();
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函数
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
     HSECFG_Capacitance(HSECap_10p);
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    // 1. Общая заводская настройка для всех портов (в Pull-Up)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    
    // 2. СВЕТОДИОДЫ (Порт В): жестко в ноль (Push-Pull), так как они сидят на GND
    GPIOB_ResetBits(LED1 | LED2);
    GPIOB_ModeCfg(LED1 | LED2, GPIO_ModeOut_PP_5mA);

   // 3. ВСЕ ЛИНИИ SPI (PA12, PA13, PA14, PA15) — ПЕРЕВОДИМ В ЖЕСТКИЙ PULL-UP!
    //Внутренняя подтяжка микроконтроллера поможет внешнему резистору R5 удержать на шине 3 Вольта,
   //что полностью закроет входные диоды акселерометра и уберет токи утечки интерфейса.
   GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    LED_GPIO_init();
    sysytem_initSystick();
    app_uart_init();
    blink_indication(10);
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
