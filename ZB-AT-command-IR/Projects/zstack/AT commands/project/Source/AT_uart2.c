/**************************************************************************************************
  Filename:       AT_uart.c

  Description:    For second uart port
  Author:         Xiao Wang
**************************************************************************************************/

#include "At_include.h"
#include "AT_uart2.h"
#include "AT_IR.h"

//void AT_UART2_IR_send(uint8 buffer[],uint8 n);
//uint8 AT_Char2Int8(uint8 num);

uint8 AT_UartInit2( uint8 taskID )
{
  (void) taskID;
  halUARTCfg_t uartConfig;


  /* UART Configuration */
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = AT_UART1_BR;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = MT_UART_THRESHOLD;
  uartConfig.rx.maxBufSize        = 64;               //actually it is useless for cc2530
  uartConfig.tx.maxBufSize        = 64;               //actually it is useless for cc2530
  uartConfig.idleTimeout          = MT_UART_IDLE_TIMEOUT;
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = AT_UartProcess2;
  return HalUARTOpen ( HAL_UART_PORT_1, &uartConfig);
}

void AT_UartProcess2( uint8 port, uint8 event ){
  uint8 code[3];
  (void) event;
  //uint8 status;
  AT_IR_t buff; 
  uint16 nwk=0x0000;
  if(Hal_UART_RxBufLen( port )){
   uint8 size=HalUARTRead(port,(uint8*)&code,5); 
   if(size==3){
   buff.cmd=AT_IR_Cmd_req;
   buff.cmdIR=UPLOAD_IR_CMD;
   //串口读取到红外模块学习的数据
   buff.code.IRversion=0x01;
   buff.code.IRlength=0x24;
   buff.code.IRtype=0x01;
   buff.code.IRaddress[0]=code[0];
   buff.code.IRaddress[1]=code[1];
   buff.code.IRvalue=code[2];
   AT_IR_Cmd_send_simple(nwk,AT_IR_CLUSTERID,sizeof(AT_IR_t),&buff);
   }
 }
}

/*void AT_UART2_IR_send(uint8 buffer[],uint8 n){
  uint8 i,j,status;
  AT_AF_IR_t buff; 
  buff.cmd=AT_AF_Cmd_req;
  buff.cmdIR=UPLOAD_IR_CMD;
  uint16 nwk=0x0000;
  //HalUARTWrite(HAL_UART_PORT_0,(uint8*)&(buffer), sizeof(buffer));
  for(i=0,j=0;i<n;i++) buff.data[j]=buffer[i];
  HalUARTWrite(HAL_UART_PORT_0,(uint8*)&(buff.data), sizeof(buff.data));
  status = AT_AF_Cmd_send_simple(nwk,AT_AF_IR_CLUSTERID,sizeof(buff),&buff);
  if(status==ZSUCCESS); //AT_OK();
}
*/
/********************************************
实现两位字符向8位整型数据转换
*********************************************/
/*
uint8 AT_Char2Int8(uint8 num){
  uint8 result=0;
  uint8 temp;
  temp=num;
  result |= _AT_ChartoInt(temp&0x0f);//取低4位转换
  temp=num;
  result |=(_AT_ChartoInt(temp>>4))<<4;//取高4位转换，并还原
  return result;
}
*/