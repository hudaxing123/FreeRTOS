
#ifndef __AT_COMMD_H
#define __AT_COMMD_H

#include "stdio.h"
#include "stdint.h"
//#include "customer_queue.h"

//队列相关
#define AT_Recive_Queue_Num  3
#define AT_Result_Queue_Num  3

#define AT_Result_Queue_Size  10

//uint8_t Recive_Buffer[BUFFER_SIZE];

/******************************************************************************
* Define the return value of AT callback function.
* RIL_ATRSP_FAILED,  means that the AT command executed failed. 
* RIL_ATRSP_SUCCESS, means that the AT command executed successfully.
* RIL_ATRSP_CONTINUE,means that continue to wait for the response of AT command.
* if return RIL_ATRSP_CONTINUE,the "Ql_RIL_SendATCmd" function will be blocked, 
* until you return to RIL_ATRSP_FAILED or RIL_ATRSP_SUCCESS.
******************************************************************************/
typedef enum {
    Callback_FAILED = 0,
    Callback_SUCCESS =1,
    Callback_CONTINUE =2,
		Callback_OTHER =3,
}Enum_ATRspError;


typedef enum {
    AT_FAILED   =   0,
    AT_SUCCESS  =   -1,
    AT_CONTINUE =   -2,
		AT_OTHER  =   -3,
}AT_commd_return;

//定义状态
typedef enum
{
	ATE1_OK = 12,
	ATI_OK,
	AT_CMEE_OK,
	AT_CIMI_OK,
	AT_CIMI_Fail
	
}ATE1_STATE;

/****************************************************************************
 * Definition for SIM Card State
 ***************************************************************************/
typedef enum {
    SIM_STAT_NOT_INSERTED = 0,
    SIM_STAT_READY,
    SIM_STAT_PIN_REQ,
    SIM_STAT_PUK_REQ,
    SIM_STAT_PH_PIN_REQ,
    SIM_STAT_PH_PUK_REQ,
    SIM_STAT_PIN2_REQ,
    SIM_STAT_PUK2_REQ,
    SIM_STAT_BUSY,
    SIM_STAT_NOT_READY,
    SIM_STAT_UNSPECIFIED
 }Enum_SIMState;

void Get_AT_Result(void);
int Handle(int (*Callback)());
void Power_ON(void);
void Get_Netstat_Result(void);
void Send_AT_Test(char *cmd);
void Recive_AT(void);
void Clean_Buffer(void);
int Send_ATE_Commd(void);
int Send_ATI_Commd(void);
int Send_AT(char *cmd,char *result, uint16_t waittime);
int ATE(void);
void Network(void);
void ATE1(void);
 
#endif


