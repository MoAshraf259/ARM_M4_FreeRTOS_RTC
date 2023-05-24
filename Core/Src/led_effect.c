/*
 * led_effect.c
 *
 *  Created on: May 21, 2023
 *      Author: moham
 */

#include "main.h"

void led_effect_stop(void){
	for(uint8_t i=0;i<4;i++){
	xTimerStop(handle_software_timers[i],portMAX_DELAY);
	}
}

void led_effect(int x){

	/*//Stop the LEDs first */
	led_effect_stop();

/*	The PC Comes here when the MenuTaskCalls the led effect
	so we start the timer to get the toggling effect desired by the user*/
	xTimerStart(handle_software_timers[x-1],portMAX_DELAY);
}

void LED_OFF_ALL(void){
	HAL_GPIO_WritePin(GPIOC, LED1, RESET);
	HAL_GPIO_WritePin(GPIOC, LED2, RESET);
	HAL_GPIO_WritePin(GPIOC, LED3, RESET);
	HAL_GPIO_WritePin(GPIOC, LED4, RESET);
}


void LED_ON_ALL(void){
	HAL_GPIO_WritePin(GPIOC, LED1, SET);
	HAL_GPIO_WritePin(GPIOC, LED2, SET);
	HAL_GPIO_WritePin(GPIOC, LED3, SET);
	HAL_GPIO_WritePin(GPIOC, LED4, SET);
}



void LED_ON_EVEN_ALL(void){
	HAL_GPIO_WritePin(GPIOC, LED1, SET);
	HAL_GPIO_WritePin(GPIOC, LED2, RESET);
	HAL_GPIO_WritePin(GPIOC, LED3, SET);
	HAL_GPIO_WritePin(GPIOC, LED4, RESET);
}

void LED_ON_ODD_ALL(void){
	HAL_GPIO_WritePin(GPIOC, LED1, RESET);
	HAL_GPIO_WritePin(GPIOC, LED2, SET);
	HAL_GPIO_WritePin(GPIOC, LED3, RESET);
	HAL_GPIO_WritePin(GPIOC, LED4, SET);
}


void LED_control( int value )
{
	 for(int i = 0 ; i < 4 ; i++)
		  HAL_GPIO_WritePin(GPIOC, (LED1 << i), ((value >> i)& 0x1));
}

void LED_effect1(void){
	static int flag=1;
	(flag ^= 1)?(LED_OFF_ALL()):(LED_ON_ALL());
}

void LED_effect2(void){
	static int flag=1;
	(flag ^= 1)?(LED_ON_EVEN_ALL()):(LED_ON_ODD_ALL());
}


void LED_effect3(void){
	static int i = 0;
	LED_control( 0x1 << (i++ % 4) );
}

void LED_effect4(void){
	static int i = 0;
	LED_control( 0x08 >> (i++ % 4) );
}
