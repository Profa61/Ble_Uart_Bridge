#include "pch.h"


uint32_t time = 0;

extern connected_status status_led;

void DigitalWrite(uint32_t gpio, GPIOMode mode)
{
    switch(mode)
        {
            case LOW:
                GPIOB_ResetBits(gpio);
                break;

            case HIGH:
                GPIOB_SetBits(gpio);
                break;

            default:
                break;
        }
}

void indicate()
{
    switch(status_led)
    {
        case NO_CONNECTED:
            if(millis() - time >= 3000)
            {
                time = millis();
                DigitalWrite(LED1, HIGH);
                TimeDelayMs(50);
                DigitalWrite(LED1, LOW);
            }
            break; // 妤快把快扶快扼快扶 我戒 if 扶忘把批忪批

        case CONNECTED:
            if(millis() - time >= 500)
            {
                time = millis();
                DigitalWrite(LED2, HIGH);
                TimeDelayMs(50);
                DigitalWrite(LED2, LOW);
            }
            break; // 妤快把快扶快扼快扶 我戒 if 扶忘把批忪批
    }
}


void blink_indication(uint8_t blink)
{
    for(uint8_t i=0; i < blink; i++)
    {
        DigitalWrite(LED2, HIGH);
        TimeDelayMs(50);
        DigitalWrite(LED2, LOW);
        TimeDelayMs(50); 
    }
}


void LED_GPIO_init()
{
    GPIOB_ResetBits(LED1);
    GPIOB_ModeCfg(LED1, GPIO_ModeOut_PP_5mA);

    GPIOB_ResetBits(LED2);
    GPIOB_ModeCfg(LED2, GPIO_ModeOut_PP_5mA);    
}







