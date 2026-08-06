#ifndef BLINK_H
#define BLINK_H




#define LED1           GPIO_Pin_0   //PB0
#define LED2           GPIO_Pin_1   //PB1
#define BUTTON         GPIO_Pin_2   //PB2
#define LIS_INT1       GPIO_Pin_1   //PA1


typedef enum
{
    LOW,
    HIGH,
}GPIOMode;

typedef enum
{
    CONNECTED,
    NO_CONNECTED
}connected_status;

void DigitalWrite(uint32_t gpio, GPIOMode mode);
void indicate();
void blink_indication(uint8_t blink);
void LED_GPIO_init();


#endif