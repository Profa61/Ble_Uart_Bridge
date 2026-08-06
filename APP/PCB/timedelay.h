#ifndef __TIMEDELAY_H__
#define __TIMEDELAY_H__


#define FREQUENCY_DIVIDER      (uint32_t)(1000)

//============================================================

void TimeDelayMs(uint32_t DelayMs);
void sysytem_initSystick(void);
void SysTick_Handler(void);
uint32_t millis();

#endif // TIMEDELAY_H
