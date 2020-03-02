/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */   

#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "at_commd.h"
//#include "Customer_queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//uint8_t Current_state;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//用于存储AT命令的返回值，通过串口传递到任务
QueueHandle_t AT_Recive_Queue;

//用于存储AT命令返回值的处理结果，通过任务传递到下一个任务
QueueHandle_t AT_Result_Queue;
 
extern UART_HandleTypeDef huart1;

extern uint8_t rx_len;
extern uint8_t recv_end_flag;
extern uint8_t rx_buffer[BUFFER_SIZE];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 100);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, StartTask02, osPriorityLow, 0, 100);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, StartTask03, osPriorityIdle, 0, 100);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	
	//创建队列，用于终中断往任务发送数据
	AT_Recive_Queue = xQueueCreate(AT_Recive_Queue_Num,BUFFER_SIZE);
	if(AT_Recive_Queue == NULL)
	{
		printf("Create AT_Recive Queue Failed!\r\n");
	}
	else
	{
		printf("Create AT_Recive Queue Succeed!\r\n");
	}
	//创建队列，用于任务与任务之间的传递，本例实现AT处理结果的通知
	AT_Result_Queue = xQueueCreate(AT_Result_Queue_Num,AT_Result_Queue_Size);
	if(AT_Result_Queue == NULL)
	{
		printf("Create AT_Result Queue Failed!\r\n");
	}
	else
	{
		printf("Create AT_Result Queue Succeed!\r\n");
	}
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
 //   uint8_t Current_state = AT_OK;	
//		printf("this is the first task1!\r\n");	
//		printf("Current_state123=%d！\r\n",Current_state);			
		printf("This is the Default Task!\r\n");
//		vTaskSuspend(NULL);
		osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */

//uint8_t Recive_Buffer[BUFFER_SIZE];
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {	
		printf("This is the Task2!\r\n");
	
		ATE1();	
	
//挂起自己，由串口中断唤醒。
			vTaskSuspend(NULL);
			osDelay(2000);
	}
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/

//用于接收队列结果
uint8_t AT_Return_Buffer[AT_Result_Queue_Size];
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    //获取AT处理结果，从AT_Result队列中获取信息
		//对信息结果做处理，并执行对应的函数
	//	Get_Netstat_Result();
		printf("This is the Task3!\r\n");	
		Network();
		osDelay(2000);
	//	vTaskSuspend(NULL);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

//void Get_Netstat_Result(void)
//{	
//	//实现输入，接收结果
////	while(1)
////	{
//	
//		uint8_t Current_state = Send_ATI_Commd();
//	
//		//获取AT返回结果，并发送新的AT命令
//		if(AT_Result_Queue != NULL)
//		{
//			
//			if(pdTRUE==xQueueReceive(AT_Result_Queue, AT_Return_Buffer,0))
//			{
//					printf("AT Result is %s\r\n",AT_Return_Buffer);
//				//if()
//			}
//			else
//			{
//				printf("Recive AT Result Fail!\r\n");
//			}		
//			
//		}
//		else
//		{
//			printf("Creat AT Result Queue Fail!\r\n");
//		}
//	
//		//从AT 返回队列中获取当前AT状态
//	
//	
//		printf("Current_state=%d！\r\n",Current_state);
//		switch(Current_state)
//		{
//			case AT_OK:
//			Current_state = Send_ATE_Commd();
//			if(Current_state == ATE1_OK)
//			{
//				Current_state = ATI_OK;
//				printf("Current_state_ATE=%d！\r\n",Current_state);
//				printf("ATE1返回成功！\r\n");
//				
//			}
//			
//			break;
//			
//			case ATI_OK:
//			Current_state = Send_ATI_Commd();
//			if(Current_state == ATI_OK)
//			{
//				printf("Current_state_ATI=%d！\r\n",Current_state);
//				Current_state = ATI_OK;
//			}
//			break;
//			
////			case ATI_OK:
////			printf("ATI返回成功！\r\n");	
////			break;	
//			
//			default:
//			printf("Report The Other URC!\r\n");
//	}
//	
//}
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
