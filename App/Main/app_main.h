/*
 * app_main.h - BeaconGalaxy application entry points (called from freertos.c).
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

void App_Init(void);
void App_Run(void);          /* never returns */

void APP_USART1_IRQ(void);   /* console */
void APP_USART2_IRQ(void);   /* link (link.c) */

#endif
