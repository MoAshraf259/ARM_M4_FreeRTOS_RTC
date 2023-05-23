/*
 * task_handler.c
 *
 *  Created on: May 21, 2023
 *      Author: moham
 */

#include "main.h"

int extract_command(command_t *cmd);
void process_command(command_t *cmd);




const char* InvMsg="INVALID MESSAGE !!";

 void menu_task(void *parameter)
{
	 uint32_t cmd_addr,option;

	 command_t *cmd;
	 const char* Menu_List="================\n"
	 				"|      MENU    |\n"
	 				"================\n"
	 				"LED effect    ----> 0\n"
	 				"Date and time ----> 1\n"
	 				"Exit          ----> 2\n"
	 				"Enter your choice here : ";

	while(1){
		 //Send this to the queue to get handled by the USART
		xQueueSend(Queue_Print,&Menu_List,portMAX_DELAY);

		//This will suspend the task until notified
		xTaskNotifyWait(0,0,&cmd_addr,portMAX_DELAY);

		//Taking the address of the COMMAND storing it in our structure variable
		cmd=(command_t*)cmd_addr;

		if(cmd->length<=1){
			/*get the first byte of the data and -48 to get the real number not ASCI*/
			option=cmd->payload[0]-48;
			switch(option){
			case 0:

				curr_state=sLedEffect;
				xTaskNotify(handle_led_task,0,eNoAction);
				break;

			case 1:

				curr_state=sRTCMenu;
				xTaskNotify(handle_rtc_task,0,eNoAction);
				break;

			case 2:
				/*DO NOTHING*/
				break;

			default:
				/*if received option isn`t within options */
				xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
				continue;
			}
		}
		else{
		/*if there is not items in queue and got notified already send invalid MSG and RELOOP*/
			xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
			continue;
		}
		/*This line will suspend the TASK again after finishing */
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

	}
}


 void command_task(void *parameter)
{
	 BaseType_t ret;
	 command_t cmd;
	while(1){
		/*Wait for notification from the USART*/
		ret =xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

		/*if notification is received then process the command received*/
		if(ret==pdTRUE){

			process_command(&cmd);
		}
	}
}
 void print_task(void *parameter)
{
	uint8_t *msg;
	while(1){
		xQueueReceive(Queue_Print, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen((char*)msg), HAL_MAX_DELAY);
	}
}
 void LED_task(void *parameter)
{
	 uint32_t cmd_addr;
	 command_t *cmd;
	 const char* msg_led = "========================\n"
					  "|      LED Effect     |\n"
					  "========================\n"
					  "(none,e1,e2,e3,e4)\n"
					  "Enter your choice here : ";

while(1){
	/* Wait for notification (Notify wait) */
	xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

	/*Print LED menu */
	xQueueSend(Queue_Print,&msg_led,portMAX_DELAY);

	/* wait for LED command (Notify wait) */
	xTaskNotifyWait(0,0,&cmd_addr,portMAX_DELAY);

	cmd=(command_t*)cmd_addr;

	if(cmd->length <= 4)
	{
		if(! strcmp((char*)cmd->payload,"none")){
			led_effect_stop();
		}
		else if (! strcmp((char*)cmd->payload,"e1")){
			led_effect(1);
		}
		else if (! strcmp((char*)cmd->payload,"e2")){
			led_effect(2);
		}
		else if (! strcmp((char*)cmd->payload,"e3")){
			led_effect(3);
		}
		else if (! strcmp((char*)cmd->payload,"e4")){
			led_effect(4);
		}
		else{
			xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
		}

	}else{
		/* print invalid message */
		xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
	}



	/*update state variable */
	curr_state = sMainMenu;

	/* Notify menu task */
	xTaskNotify(handle_menu_task,0,eNoAction);

	}

}

 uint8_t getnumber(uint8_t *p , int len)
 {

 	int value ;

 	if(len > 1)
 	   value =  ( ((p[0]-48) * 10) + (p[1] - 48) );
 	else
 		value = p[0] - 48;

 	return value;

 }

 void RTC_Task(void *parameter)
{
		const char* msg_rtc1 = "========================\n"
								"|         RTC          |\n"
								"========================\n";

		const char* msg_rtc2 = "Configure Time            ----> 0\n"
								"Configure Date            ----> 1\n"
								"Enable reporting          ----> 2\n"
								"Exit                      ----> 3\n"
								"Enter your choice here : ";


		const char *msg_rtc_hh = "Enter hour(1-12):";
		const char *msg_rtc_mm = "Enter minutes(0-59):";
		const char *msg_rtc_ss = "Enter seconds(0-59):";

		const char *msg_rtc_dd  = "Enter date(1-31):";
		const char *msg_rtc_mo  ="Enter month(1-12):";
		const char *msg_rtc_dow  = "Enter day(1-7 sun:1):";
		const char *msg_rtc_yr  = "Enter year(0-99):";

		const char *msg_conf = "Configuration successful\n";
		const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


		uint32_t cmd_addr;
		command_t *cmd;

		static int rtc_state = 0;
		int menu_code;

		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;


	#define HH_CONFIG 		0
	#define MM_CONFIG 		1
	#define SS_CONFIG 		2

	#define DATE_CONFIG 	0
	#define MONTH_CONFIG 	1
	#define YEAR_CONFIG 	2
	#define DAY_CONFIG 		3


		while(1){
			/*Notify wait (wait till someone notifies) */
			xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

			/*Print the menu and show current date and time information */
			xQueueSend(Queue_Print,&msg_rtc1,portMAX_DELAY);
			show_time_date();
			xQueueSend(Queue_Print,&msg_rtc2,portMAX_DELAY);


			while(curr_state != sMainMenu){

				/*Wait for command notification (Notify wait) */
				xTaskNotifyWait(0,0,&cmd_addr,portMAX_DELAY);
				cmd = (command_t*)cmd_addr;

				switch(curr_state)
				{

					case sRTCMenu:{
						/*process RTC menu commands */
						if(cmd->length == 1)
						{
							menu_code = cmd->payload[0] - 48;
							switch(menu_code)
							{
							case 0:
								curr_state = sRTCTimeConfig;
								xQueueSend(Queue_Print,&msg_rtc_hh,portMAX_DELAY);
								break;
							case 1:
								curr_state = sRTCDateConfig;
								xQueueSend(Queue_Print,&msg_rtc_dd,portMAX_DELAY);
								break;
							case 2 :
								curr_state = sRTCReport;
								xQueueSend(Queue_Print,&msg_rtc_report,portMAX_DELAY);
								break;
							case 3 :
								curr_state = sMainMenu;
								break;
							default:
								curr_state = sMainMenu;
								xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
							}

						}else{
							curr_state = sMainMenu;
							xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
						}
						break;}

					case sRTCTimeConfig:{
						/*get hh, mm, ss infor and configure RTC */
						/*take care of invalid entries */
						switch(rtc_state)
							{
								case HH_CONFIG:{
									uint8_t hour = getnumber(cmd->payload , cmd->length);
									time.Hours = hour;
									rtc_state = MM_CONFIG;
									xQueueSend(Queue_Print,&msg_rtc_mm,portMAX_DELAY);
									break;}
								case MM_CONFIG:{
									uint8_t min = getnumber(cmd->payload , cmd->length);
									time.Minutes = min;
									rtc_state = SS_CONFIG;
									xQueueSend(Queue_Print,&msg_rtc_ss,portMAX_DELAY);
									break;}
								case SS_CONFIG:{
									uint8_t sec = getnumber(cmd->payload , cmd->length);
									time.Seconds = sec;
									if(!validate_rtc_information(&time,NULL))
									{
										rtc_configure_time(&time);
										xQueueSend(Queue_Print,&msg_conf,portMAX_DELAY);
										show_time_date();
									}else
										xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);

									curr_state = sMainMenu;
									rtc_state = 0;
									break;}
							}

						break;}

					case sRTCDateConfig:{

						/*get date, month, day , year info and configure RTC */

						/*take care of invalid entries */
						switch(rtc_state)
							{
								case DATE_CONFIG:{
									uint8_t d = getnumber(cmd->payload , cmd->length);
									date.Date = d;
									rtc_state = MONTH_CONFIG;
									xQueueSend(Queue_Print,&msg_rtc_mo,portMAX_DELAY);
									break;}
								case MONTH_CONFIG:{
									uint8_t month = getnumber(cmd->payload , cmd->length);
									date.Month = month;
									rtc_state = DAY_CONFIG;
									xQueueSend(Queue_Print,&msg_rtc_dow,portMAX_DELAY);
									break;}
								case DAY_CONFIG:{
									uint8_t day = getnumber(cmd->payload , cmd->length);
									date.WeekDay = day;
									rtc_state = YEAR_CONFIG;
									xQueueSend(Queue_Print,&msg_rtc_yr,portMAX_DELAY);
									break;}
								case YEAR_CONFIG:{
									uint8_t year = getnumber(cmd->payload , cmd->length);
									date.Year = year;

									if(!validate_rtc_information(NULL,&date))
									{
										rtc_configure_date(&date);
										xQueueSend(Queue_Print,&msg_conf,portMAX_DELAY);
										show_time_date();
									}else
										xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);

									curr_state = sMainMenu;
									rtc_state = 0;
									break;}
							}


						break;}

					case sRTCReport:{
						/*enable or disable RTC current time reporting over ITM printf */
						if(cmd->length == 1)
						{
							if(cmd->payload[0] == 'y'){
								if(xTimerIsTimerActive(rtc_timer) == pdFALSE)
									xTimerStart(rtc_timer,portMAX_DELAY);
							}else if (cmd->payload[0] == 'n'){
								xTimerStop(rtc_timer,portMAX_DELAY);
							}else{
								xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);
							}

						}else
						    xQueueSend(Queue_Print,&InvMsg,portMAX_DELAY);

						curr_state = sMainMenu;
						break;}

				}// switch end

			} //while end


			/*Notify menu task */
			xTaskNotify(handle_menu_task,0,eNoAction);

			}//while super loop end
	}


 void process_command(command_t *cmd){

	 extract_command(cmd);

	 switch(curr_state){
	 case sMainMenu:
		 xTaskNotify(handle_menu_task,(uint32_t)cmd,eSetValueWithOverwrite);
		 break;

	 case sLedEffect:
		 xTaskNotify(handle_led_task,(uint32_t)cmd,eSetValueWithOverwrite);
		 break;

	 case sRTCMenu:
	 case sRTCTimeConfig:
	 case sRTCDateConfig:
	 case sRTCReport:
		 xTaskNotify(handle_rtc_task,(uint32_t)cmd,eSetValueWithOverwrite);
		 break;
	 }
 }

int extract_command(command_t *cmd){

	 uint8_t item;
	 BaseType_t status;

	 /*Check if there is any data available in queue
	 if there is data available in the QUEUE function will return pdTrue*/
	 status=uxQueueMessagesWaiting(Queue_Data);

	 if(status==pdFALSE){
		 return -1;
	 }
	 uint8_t i=0;

	 do{
		 /*Take the data from queue into the variable item*/
		 status=xQueueReceive(Queue_Data, (void*)&item, 0);
		 if(status ==pdTRUE){
			 cmd->payload[i++]=item;
		 }
	 }while(item !='\n');

	 cmd->payload[i-1]='\0';/*Switch the new line character with NULL Character*/
	 cmd->length=i-1;
	 return 0;

 }
