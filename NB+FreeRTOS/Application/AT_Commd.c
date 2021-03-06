
#include "stdlib.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "at_commd.h"
#include "cmsis_os.h"
//#include "customer_queue.h"


//#define  ATE4_OK    2

uint8_t AT_Return_Queue[AT_Result_Queue_Size];

//uint8_t rx_buffer[BUFFER_SIZE]={0};

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern uint8_t rx_len;
extern uint8_t recv_end_flag;
extern uint8_t rx_buffer[BUFFER_SIZE];


extern QueueHandle_t AT_Recive_Queue;
extern QueueHandle_t AT_Result_Queue;
uint8_t Recive_Buffer[BUFFER_SIZE];

//清除BUFFER 缓存
void Clean_Buffer(void)
{
	for(uint8_t i=0;i<rx_len;i++)
		{
			rx_buffer[i]=0;//清接收缓存
		}
	rx_len=0;//清除计数
	recv_end_flag=0;//清除接收结束标志位
	HAL_UART_Receive_DMA(&huart2,rx_buffer,BUFFER_SIZE);//重新打开DMA接收			
}

/*
******* power_on & Reset**********
******* reboot the module *****
*****************
************
*/
void Power_ON(void)
{
	BaseType_t Recive_status;
	BaseType_t TaskWoken = pdFALSE;
	if(AT_Recive_Queue != NULL)
		{
			Recive_status = xQueueReceiveFromISR(AT_Recive_Queue, Recive_Buffer,&TaskWoken);
			if(Recive_status == pdTRUE)
			{
				printf("Recive is: %s\r\n",Recive_Buffer);
				//对接收到的数据进行处理，处理完将结果发送到队列并通知任务3
				if(strstr((char *)Recive_Buffer,"REBOOT_CAUSE_SECURITY_RESET_PIN"))
					{		
						HAL_UART_Transmit(&huart1,Recive_Buffer, rx_len,1025);//接收数据打印出来
						printf("Reboot by Reset key!\r\n");
						
					}
				else if(strstr((char *)Recive_Buffer,"REBOOT_CAUSE_SECURITY_RESET_PIN"))
					{		
						HAL_UART_Transmit(&huart1,Recive_Buffer, rx_len,1025);//接收数据打印出来
						printf("Reboot by !\r\n");
						
					}
				else
					{
						//队列中接受到了其他的数据内容
						printf("AT return Fail!\r\n");
					//	result = 0;
					}
			}
			else
			{
				printf("Waitting the Queue data!\r\n");
			//	result = 0;
			}
			}	
		else
		{
				printf("Creat the queue Fail!\r\n");
		}				
}



int AT_Callback (char *AtReturn, char *Result)
{
	char AT_result = 0;
//	char *p;
//	BaseType_t Recive_status;
		uint8_t AT[15];
		uint8_t i = 0;
		uint8_t t = 0;
	BaseType_t TaskWoken = pdFALSE;
	if(AT_Recive_Queue != NULL)
		{
	//		printf("AT_Recive_Queue已用长度：%d\r\n",(int)uxQueueMessagesWaiting(AT_Recive_Queue));
			if(xQueueReceiveFromISR(AT_Recive_Queue, Recive_Buffer,&TaskWoken) == pdTRUE)
			{
//				printf("AT_Recive_Queue用了长度：%d\r\n",(int)uxQueueMessagesWaiting(AT_Recive_Queue));
//				printf("接收的数据是: %s\r\n",Recive_Buffer);
				//对接收到的数据进行处理，处理完将结果发送到队列并通知任务3
				if(strstr((char *)Recive_Buffer,(char *)AtReturn))
					{		
						while(!((Recive_Buffer[i]=='\r')&&(Recive_Buffer[i+1]=='\n')))   //查找换行符
						{
							i++;	
						}
//						printf("AT Recive len is :%d\r\n",i);
//						printf("AT Recive is :%s\r\n",(char *)Recive_Buffer);
						for(t=0;t<15;t++)
							{
								AT[t]=Recive_Buffer[i+2];
								i++;
							}
//							printf("AT1 指令返回结果：%s\r\n",(char *)AT);
							if(strstr((char *)AT,(char *)Result))
							{
//							printf("AT1 指令返回结果返回成功\r\n");
								printf("AT commd return Success!\r\n");
								AT_result = 1;
							}
				//		HAL_UART_Transmit(&huart1,Recive_Buffer, rx_len,1025);//接收数据打印出来
					}
				else
					{
						//队列中接受到了其他的数据内容
						printf("AT return Fail!\r\n");
						AT_result = 0;
					}
			}
			else
			{
				printf("Waitting the Queue data!\r\n");
				AT_result = 0;
			}
			}	
		else
		{
				printf("Creat the queue Fail!\r\n");
				AT_result = 0;
		}						
	 return AT_result;		
}


//Return 0 is Scusfull, Return 1 is Failed!
int Send_AT(char *cmd, char *Atreturn, char *result, uint16_t waittime)
{
	int res = 0;
	HAL_UART_Transmit(&huart2,(uint8_t*)cmd,strlen(cmd),0xFFFF);
	HAL_UART_Transmit(&huart2,(uint8_t*)"\r",1,100);
	HAL_UART_Transmit(&huart2,(uint8_t*)"\n",1,100);
	printf("Send AT is %s!\r\n",cmd);
	
	if(result && waittime)
	{	
		while(--waittime)
		{
//			printf("Waitting AT Return ………!\r\n");
//			printf("Waitting times is :%d\r\n",waittime);				
			vTaskDelay(10);
			if(AT_Callback (Atreturn,result))break;
			if(waittime==1)
			{
				printf("Waitting AT Return Timeout!\r\n");		
				res = 1;
			}
		}
		memset(Recive_Buffer,0,BUFFER_SIZE);    //清除缓存区		
	}	
	else
	{
		printf("Other……!\r\n");		
		res = 1;
	}
	
	return res;
}
	
//uint8_t ATE_Return[1]={0};
//Echo mode ON   开启回显
void ATE1(void)
{
//	uint8_t ATE2_OK = 12;
	uint8_t ATE1OK = ATE1_OK;

	if(!(Send_AT("ATE1","ATE1","OK",30)))
	{
		//往结果队列中传入成功
	if( AT_Result_Queue != 0 )
	{
		// Send an uint32_t.  Wait for 10 ticks for space to become
		// available if necessary.
		if( xQueueSend( AT_Result_Queue, (void *)&ATE1OK, ( TickType_t ) 10 ) == pdPASS )
		{
			printf("ATE1 Return Success Send to queue………!\r\n");
	//		printf("AT_Result_Queue 已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
		}
		else
		{
			printf("AT_Result_Queue is Full!\r\n");
		}
	}
	}
	
	else
	{
		printf("Send ATE1 Commd Failed!\r\n");
		//往结果队列中传入
	}
}
void ATI(void)
{
	uint8_t ATIOK = ATI_OK;
	if(!(Send_AT("ATI", "ATI","Quectel", 30)))
	{
		//往结果队列中传入成功
	if( AT_Result_Queue != 0 )
	{
		// Send an uint32_t.  Wait for 10 ticks for space to become
		// available if necessary.
		if( xQueueSend( AT_Result_Queue, (void *)&ATIOK, ( TickType_t ) 10 ) == pdPASS )
		{
			printf("ATI Return Success Send to queue………!\r\n");
	//		printf("AT_Result_Queue 已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
		}
		else
		{
			printf("AT_Result_Queue is Full!\r\n");
		}
	}	
	}
	else
	{
		//往结果队列中传入失败
	//	if(!(Send_AT("ATE1", "ERROR", 30)))
		printf("Send ATI Commd Failed!\r\n");
	}
}
void AT_CMEE(void)
{
	uint8_t CMEEOK = AT_CMEE_OK;
	if(!(Send_AT("AT+CMEE=1","AT+CMEE=1","OK",30)))
	{
		//往结果队列中传入成功
	if( AT_Result_Queue != 0 )
	{
		// Send an uint32_t.  Wait for 10 ticks for space to become
		// available if necessary.
		if( xQueueSend( AT_Result_Queue, (void *)&CMEEOK, ( TickType_t ) 10 ) == pdPASS )
		{
			printf("AT+CMEE=1 Return Success Send to queue………!\r\n");
	//		printf("AT_Result_Queue 已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
		}
		else
		{
			printf("AT_Result_Queue is Full!\r\n");
		}
	}
	
	}
	else
	{
		//往结果队列中传入失败
	}

}
void AT_CIMI(void)
{
	uint8_t CIMIOK = AT_CIMI_OK;
	uint8_t CIMIFail = AT_CIMI_Fail;
	if(!(Send_AT("AT+CIMI", "AT+CIMI","4600",30)))
	{
		printf("AT+CIMI do finsh!\r\n");
		//往结果队列中传入成功
	if( AT_Result_Queue != 0 )
	{
		// Send an uint32_t.  Wait for 10 ticks for space to become
		// available if necessary.
		if( xQueueSend( AT_Result_Queue, (void *)&CIMIOK,0) == pdPASS )
		{
			printf("AT+CIMI Return OK and Success Send to queue………!\r\n");
	//		printf("AT_Result_Queue 已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
		}
		else
		{
			printf("AT_Result_Queue is Full!\r\n");
		}
	}
	}
	else if(!(Send_AT("AT+CIMI","AT+CIMI","+CME ERROR:", 30)))
	{
		printf("AT+CIMI do1 finsh!\r\n");
	if( AT_Result_Queue != 0 )
	{
		// Send an uint32_t.  Wait for 10 ticks for space to become
		// available if necessary.
		if( xQueueSend( AT_Result_Queue, (void *)&CIMIFail,0) == pdPASS )
		{
			printf("AT+CIMI Return Fail and Success Send to queue………!\r\n");
	//		printf("AT_Result_Queue 已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
		}
		else
		{
			printf("AT_Result_Queue is Full!\r\n");
		}
	}
		
	}
//	else
//	{
//		//往结果队列中传入失败
//		printf("AT_Result_Queue is Full!\r\n");
//	}
}
//void CFUN(void)
//{
//	if(!(Send_AT("AT+CFUN�?", "+CFUN:", 30)))
//	{
//		//往结果队列中传入成功	
//	}
//	else if(Send_AT("AT+CFUN�?", "+CME ERROR:", 30))
//	{
//		//往结果队列中传入成功，错误代码
//	}	
//	else
//	{
//		//往结果队列中传入失败
//	}
//}

uint8_t Return_Status = 0;

void Network(void)
{
//	int Return_Status = 0;	  //get the "at result" queue 
//	uint8_t Return_Status = 0;      //为什么放在这里不行，会不执行该函数
	if( AT_Result_Queue != 0 )
	{
		if( xQueueReceive( AT_Result_Queue, (void *)&Return_Status,0)== pdTRUE )
		{
			printf("AT Result Status is :%d\r\n",Return_Status);
	//		printf("AT_Result_Queue已用长度：%d\r\n",(int)uxQueueMessagesWaiting( AT_Result_Queue));
//		return Return_Status;
		}
		else
		{
			printf("AT Result Status Queue Recive Failed!\r\n");
		}
	}
	else
	{
		printf("AT Result Status Queue is empty!\r\n");
	}
	
	switch(Return_Status)
	{
	   case ATE1_OK:
        printf("ATE1 执行成功!\r\n");	
				ATI();
          break;
		 case ATI_OK:
        printf("ATI 执行成功!\r\n");
				AT_CMEE();
          break;	
		 case AT_CMEE_OK:
        printf("AT+CMEE=1 执行成功!\r\n");
				AT_CIMI();
//				Return_Status = 0;
          break;	
		 case AT_CIMI_OK:
        printf("AT+CIMI执行成功!\r\n");
//				AT_CIMI();
//				Return_Status = 0;
          break;	
		 case AT_CIMI_Fail:
        printf("AT+CIMI执行失败!\r\n");
				AT_CIMI();
//				Return_Status = 0;
          break;	
		 
		  default:
          break;	
	}
	
//	return "was";
}
/*
//检查返回的响应是否符合预期
//传入参数为预期返回的字符串
//返回0，为检测到预期值
//其他值，预期字符所在的位置
uint8_t* BC95_check_ack(char *str)
{
	char *strx=0;
	if(usart2_rcvd_flag)		
	{ 
		usart2_rcvd_buf[usart2_rcvd_len]='\0';
		strx=strstr((const char*)usart2_rcvd_buf,(const char*)str);
	} 
	return (uint8_t*)strx;
}

//发生at指令函数
//cmd:at指令，ack：预期响应，waittime,超时时间
//返回0，发送成功
//返回1，发送超时
uint8_t BC95_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	uint8_t res=0; 
	usart2_rcvd_flag=0;
	usart2_rcvd_len = 0;
	memset(usart2_rcvd_buf,0,USART2_RX_BUF_LEN);
	printf("%s\r\n",cmd);
	if(ack&&waittime)
	{
		while(--waittime)	
		{
			delay_ms(20);
			if(usart2_rcvd_flag)
			{
				
				if(BC95_check_ack(ack))break;
				usart2_rcvd_flag=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
*/




/*
int32_t atcmd_send(AT_CMD current_atcmd,void* userData)
{
 int32_t ret=AT_PRO_GOT_RESULT;
 uint8_t len;
 current_at_state = RIL_AT_WAIT;
 printf("\r\n");
 printf("atcmd_send_log:%s",current_atcmd.pcmd);
//  send ATcmd
 //__HAL_UART_DISABLE_IT(&Uart_Nb_Handle,UART_IT_RXNE);
 ret=HAL_UART_Transmit(&Uart_Nb_Handle,(uint8_t*)(current_atcmd.pcmd),strlen(current_atcmd.pcmd),0xffff);
 //__HAL_UART_ENABLE_IT(&Uart_Nb_Handle,UART_IT_RXNE);
 if (ret)
 {
  printf("return -2!!:%d\r\n",ret);
  return RIL_AT_INVALID_PARAM;
 }
 HAL_Delay(100);
 //printf("HAL_Delay(100);\r\n");
 if(current_atcmd.pprefix&&current_atcmd.timeout)
 {
  again:
  while(--current_atcmd.timeout)
  {
   //printf("--current_atcmd.timeout\r\n");
   if(USART_RX_STA&0x8000)
   {
    len=USART_RX_STA&0x3fff;//Get the length of the received data
    ret = current_atcmd.atcmd_result_callback((char*)USART_RX_BUF, len, userData);
    //printf("USART_RX_BUF:%s\r\n",(uint8_t *)USART_RX_BUF);
    USART_RX_STA=0;
    memset((uint8_t *)USART_RX_BUF,0x00,200);
    break;
   }
   HAL_Delay(10);
  }
  if(0==current_atcmd.timeout)
  {
   printf("current_atcmd.timeout!!\r\n");
   return RIL_AT_TIMEOUT;
  }
 }
 if(ret==RIL_ATRSP_CONTINUE)
 {
  goto again;
 }
 else if(ret==RIL_ATRSP_FAILED)
 {
  return RIL_AT_FAILED;
 }
 else
 {
  #ifdef DBG_MODE
  printf("return RIL_AT_SUCCESS!!\r\n");
  #endif
  return RIL_AT_SUCCESS;
 }
}
*/


