/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "AT_UART.h"
#include "AT_AF.h"

#include "GenericApp.h"
#include "DebugTrace.h"

#include "OnBoard.h"

/* HAL */
#include "hal_led.h"
#include "hal_uart.h"

/*********************************************************************
 * MACROS
 */
#define HAL_LED_OFF   1
#define HAL_LED_ON    0

// This is the max byte count per OTA message.

/*********************************************************************
 * LOCAL VARIABLES
 */
devStates_t GenericApp_NwkState;

byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.
/*********************************************************************
 * LOCAL FUNCTIONS
 */
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void GenericApp_Init( byte task_id )
{
  
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  
  AT_AF_init();
  AT_UART_init();
}

/*********************************************************************
 * @fn      GenericApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;
    
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD:
                GenericApp_MessageMSGCB(MSGpkt);
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  byte data ;
     
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
     data  = pkt->cmd.Data[0];    //osal_memcpy(&data, pkt->cmd.Data, 1);
     
     if(data == 0x10){
       HalLedSet(HAL_LED_1, HAL_LED_ON); 
     }
     else if(data == 0x11){
      HalLedSet(HAL_LED_1, HAL_LED_OFF); 
     }
     else if(data == 0x20){
       HalLedSet(HAL_LED_2, HAL_LED_ON);
     }
     else if(data == 0x21){
      HalLedSet(HAL_LED_2, HAL_LED_OFF);
     }
     else if(data == 0x30){
       HalLedSet(HAL_LED_3, HAL_LED_ON);
     }
     else if(data == 0x31){
       HalLedSet(HAL_LED_3, HAL_LED_OFF); 
     }
      break;
  }
   HalUARTWrite(HAL_UART_PORT_0,&data,1);
}

