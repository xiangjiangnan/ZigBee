#include "hal_uart.h"
#include "AT_AF.h"
#include "AT_UART.h"
#include "OnBoard.h"

char dat;

void rxCB(uint8 port,uint8 event);

void AT_UART_init(void){
  
halUARTCfg_t uartConfig;
  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = HAL_UART_BR_115200;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = MT_UART_THRESHOLD;   // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = AT_UART_RX_BUFF_MAX;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = AT_UART_TX_BUFF_MAX;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = MT_UART_IDLE_TIMEOUT;    // 在idleTimout 时间内RX还没有得到新的数据，将会触发相应的事件HAL_UART_RX_TIMEOUT ，
                                                            //这时应用可以选择读出所有RX的值或者一部分。
  uartConfig.intEnable            = TRUE; // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = rxCB;
  
  HalUARTOpen (HAL_UART_PORT_0, &uartConfig); 
}
void rxCB(uint8 port,uint8 event){
      uint8 len = HalUARTRead(port, &dat,1);
      if(len){
      GenericApp_SendTheMessage(dat);
      HalUARTWrite(port,RxBuf,len);
      len = 0;
      }
      
}

