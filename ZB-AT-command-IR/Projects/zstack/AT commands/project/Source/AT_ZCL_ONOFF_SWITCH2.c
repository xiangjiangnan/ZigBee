
/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ms.h"
#include "zcl_ha.h"
#include "AT_ZCL.h"
#include "AT_uart.h"
#include "AT_ZCL_ONOFF_SWITCH2.h"
#include "AT_switch2.h"

#include "onboard.h"

/* HAL */
#include "hal_led.h"
#include "hal_key.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte AT_ZCL_ONOFF_SWITCH2_TaskID;

/*********************************************************************
 * LOCAL FUNCTION
 */
static void AT_ZCL_ONOFF_SWITCH2_IdentifyCB( zclIdentify_t *pCmd );
static void AT_ZCL_ONOFF_SWITCH2_BasicResetCB( void );
static void AT_ZCL_ONOFF_SWITCH2_ProcessIdentifyTimeChange( void );
static void AT_ZCL_ONOFF_SWITCH2_update(void);
static void AT_ZCL_ONOFF_SWITCH2_OnOffCB( uint8 cmd );
static void AT_ZCL_ONOFF_SWITCH2_EP_ENABLE( bool isEnable);

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t AT_ZCL_ONOFF_SWITCH2_GEN_CmdCallbacks =
{ 
  AT_ZCL_ONOFF_SWITCH2_BasicResetCB,                 // Basic Cluster Reset command
  AT_ZCL_ONOFF_SWITCH2_IdentifyCB,                   // Identify command   
  NULL,                                             // Identify Query Response command
  AT_ZCL_ONOFF_SWITCH2_OnOffCB,                      // On/Off cluster command
  NULL,                                     // Level Control Move to Level command
  NULL,                                     // Level Control Move command
  NULL,                                     // Level Control Step command
  NULL,                                     // Group Response commands
  NULL,                                     // Scene Store Request command
  NULL,                                     // Scene Recall Request command
  NULL,                                     // Scene Response command
  NULL,                                     // Alarm (Response) command
  NULL,                                     // RSSI Location commands
  NULL,                                     // RSSI Location Response commands
};


void AT_ZCL_ONOFF_SWITCH2_Init( byte task_id )
{
  AT_ZCL_ONOFF_SWITCH2_TaskID = task_id;

  // This app is part of the Home Automation Profile
  zclHA_Init( &AT_ZCL_ONOFF_SWITCH2_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( AT_ZCL_ONOFF_SWITCH2_ENDPOINT, &AT_ZCL_ONOFF_SWITCH2_GEN_CmdCallbacks );
  
  // Register the application's attribute list
  zcl_registerAttrList( AT_ZCL_ONOFF_SWITCH2_ENDPOINT, AT_ZCL_ONOFF_SWITCH2_MAX_ATTRIBUTES, AT_ZCL_ONOFF_SWITCH2_Attrs );
  
  //register for AT command system enable/disable call back function
  AT_ZCL_EP_ENABLE_Register(  AT_ZCL_ONOFF_SWITCH2_ENDPOINT,AT_ZCL_ONOFF_SWITCH2_EP_ENABLE);
  
  //initialize the switch2 device
  switch2_init();
  
  
}



/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 AT_ZCL_ONOFF_SWITCH2_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( AT_ZCL_ONOFF_SWITCH2_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      { 
         default:
          break;
      }
      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & AT_ZCL_ONOFF_SWITCH2_IDENTIFY_TIMEOUT_EVT )
  {
    if (AT_ZCL_ONOFF_SWITCH2_IdentifyTime > 0 )
      AT_ZCL_ONOFF_SWITCH2_IdentifyTime--;
    AT_ZCL_ONOFF_SWITCH2_ProcessIdentifyTimeChange();

    return ( events ^ AT_ZCL_ONOFF_SWITCH2_IDENTIFY_TIMEOUT_EVT );
  }
  
  //updata the switch2 status
  if ( events & AT_ZCL_ONOFF_SWITCH2_UPDATE_EVT )
  {
    AT_ZCL_ONOFF_SWITCH2_update();
    osal_start_timerEx( AT_ZCL_ONOFF_SWITCH2_TaskID, AT_ZCL_ONOFF_SWITCH2_UPDATE_EVT, 500 );
    return ( events ^ AT_ZCL_ONOFF_SWITCH2_UPDATE_EVT );
  }
  
  // Discard unknown events
  return 0;
}




/*********************************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_update
 *
 * @brief   updata the Device Temperature Configuration Cluster every second
 *
 * @param   none
 *
 * @return  none
 *
************************************************************/
static void AT_ZCL_ONOFF_SWITCH2_update(void){
  uint8 sw = switch2_status();
  if(AT_ZCL_ONOFF_SWITCH2_action != sw){
    AT_ZCL_ONOFF_SWITCH2_action=sw;
    
    afAddrType_t dstAddr;
    dstAddr.addrMode = afAddrNotPresent;
    if(AT_ZCL_ONOFF_SWITCH2_action==ON_OFF_SWITCH_ACTIONS_0 ){
      zclGeneral_SendOnOff_CmdOff(AT_ZCL_ONOFF_SWITCH2_ENDPOINT,&dstAddr,1,1);
    }else{
      zclGeneral_SendOnOff_CmdOn(AT_ZCL_ONOFF_SWITCH2_ENDPOINT,&dstAddr,1,1);
    }
    
  }
  
  if(AT_ZCL_ONOFF_SWITCH2_action==ON_OFF_SWITCH_ACTIONS_0) 
      HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
  else HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
  
}




/*********************************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void AT_ZCL_ONOFF_SWITCH2_BasicResetCB( void )
{
  // Reset all attributes to default values
  AT_ZCL_ONOFF_SWITCH2_OnOffCB(COMMAND_OFF);
  
  zclIdentify_t identifyCmd={NULL,0};
  AT_ZCL_ONOFF_SWITCH2_IdentifyCB( &identifyCmd );
}


/*********************************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   srcAddr - source address and endpoint of the response message
 * @param   identifyTime - the number of seconds to identify yourself
 *
 * @return  none
 */
static void AT_ZCL_ONOFF_SWITCH2_IdentifyCB( zclIdentify_t *pCmd )
{
  AT_ZCL_ONOFF_SWITCH2_IdentifyTime = pCmd->identifyTime;
  AT_ZCL_ONOFF_SWITCH2_ProcessIdentifyTimeChange();
}



/*********************************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_OnOffCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   cmd - COMMAND_ON, COMMAND_OFF or COMMAND_TOGGLE
 *
 * @return  none
 */
static void AT_ZCL_ONOFF_SWITCH2_OnOffCB( uint8 cmd )
{
  // Turn on the light
  if ( cmd == COMMAND_ON )
    AT_ZCL_ONOFF_SWITCH2_OnOff = AT_ZCL_GEN_ON;

  // Turn off the light
  else if ( cmd == COMMAND_OFF )
    AT_ZCL_ONOFF_SWITCH2_OnOff = AT_ZCL_GEN_OFF;

  // Toggle the light
  else
  {
    if ( AT_ZCL_ONOFF_SWITCH2_OnOff == AT_ZCL_GEN_OFF )
      AT_ZCL_ONOFF_SWITCH2_OnOff = AT_ZCL_GEN_ON;
    else
      AT_ZCL_ONOFF_SWITCH2_OnOff = AT_ZCL_GEN_OFF;
  }

  // In this sample app, we use LED2 to simulate
  if ( AT_ZCL_ONOFF_SWITCH2_OnOff == AT_ZCL_GEN_ON ){
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    osal_start_timerEx( AT_ZCL_ONOFF_SWITCH2_TaskID, AT_ZCL_ONOFF_SWITCH2_UPDATE_EVT, 10 );
  }
  else{
    HalLedSet( HAL_LED_2, HAL_LED_MODE_OFF );
    osal_stop_timerEx( AT_ZCL_ONOFF_SWITCH2_TaskID, AT_ZCL_ONOFF_SWITCH2_UPDATE_EVT );
    AT_ZCL_ONOFF_SWITCH2_action=0xFF;   //0xFF indicates that the value is invalid.
  }
}


/*********************************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   none
 *
 * @return  none
 */
static void AT_ZCL_ONOFF_SWITCH2_ProcessIdentifyTimeChange( void )
{
  if ( AT_ZCL_ONOFF_SWITCH2_IdentifyTime > 0 )
  {
    osal_start_timerEx( AT_ZCL_ONOFF_SWITCH2_TaskID, AT_ZCL_ONOFF_SWITCH2_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_2, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    if ( AT_ZCL_ONOFF_SWITCH2_OnOff )
      HalLedSet ( HAL_LED_2, HAL_LED_MODE_ON );
    else
      HalLedSet ( HAL_LED_2, HAL_LED_MODE_OFF );
    osal_stop_timerEx( AT_ZCL_ONOFF_SWITCH2_TaskID, AT_ZCL_ONOFF_SWITCH2_IDENTIFY_TIMEOUT_EVT );
   }
}

/******************************************************
 * @fn      AT_ZCL_ONOFF_SWITCH2_EP_ENABLE
 *
 * @brief   Process  Measurement and Sensing profile call Callback function
********************************************************/
static void AT_ZCL_ONOFF_SWITCH2_EP_ENABLE( bool isEnable){
  if(isEnable) {
    AT_ZCL_ONOFF_SWITCH2_OnOffCB(COMMAND_ON);
  }
  else AT_ZCL_ONOFF_SWITCH2_BasicResetCB();
}
