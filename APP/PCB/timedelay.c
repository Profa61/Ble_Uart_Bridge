#include "CH58x_common.h"
#include "pch.h"


volatile uint32_t timestamp = 0;

//===================================================================================
// §ª§ß§Ú§è§Ú§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ SysTick

void sysytem_initSystick(void)
{
    PFIC_EnableIRQ(SysTick_IRQn);  // §£§Ü§Ý§ð§é§Ñ§Ö§Þ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Ö SysTick
    SysTick_Config(FREQ_SYS / FREQUENCY_DIVIDER);
}

//===================================================================================

/*§¨§Õ§Ö§ä §ß§Ñ §Ó§ç§à§Õ§Ö §Ù§Ñ§Õ§Ö§â§Ø§Ü§å §Ó §®§³
 *§¯§Ñ §Ó§í§ç§à§Õ§Ö §â§Ö§Ù§å§Ý§î§ä§Ñ§ä bool
 */

void TimeDelayMs(uint32_t DelayMs)
{
    uint32_t start_time = timestamp;

    while(1)
    {
        uint32_t current_time = timestamp;

        if( (current_time - start_time) >= DelayMs )
        {
            break;
        }
    }
}


//=====================================================================================
uint32_t millis()
{
    return timestamp;
}

//======================================================================================



void SysTick_Handler(void)__attribute__ ((interrupt("WCH-Interrupt-fast"))); // A§ä§ä§â§Ú§Ò§å§ä §Õ§Ý§ñ §Ó§í§Ù§à§Ó§Ñ §à§Ò§â§Ñ§Ò§à§ä§é§Ú§Ü§Ñ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ
void SysTick_Handler(void)
{
    timestamp++;
    SysTick->SR = 0;   // §°§é§Ú§ë§Ñ§Ö§Þ §æ§Ý§Ñ§Ô §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ
}
