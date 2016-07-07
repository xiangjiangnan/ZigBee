
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
#include "AT_ZCL_DHT.h"
#include "AT_DHT11.h" 

#include "onboard.h"

/* HAL */
#include "hal_led.h"
#include "hal_key.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte AT_ZCL_DHT_TaskID;

/*********************************************************************
 * LOCAL FUNCTION
 */
static void AT_ZCL_DHT_IdentifyCB( zclIdentify_t *pCmd );
static void AT_ZCL_DHT_BasicResetCB( void );
static void AT_ZCL_DHT_ProcessIdentifyTimeChange( void );
static void AT_ZCL_DHT_OnOffCB( uint8 cmd );
static void AT_ZCL_DHT_update(void);
static void AT_ZCL_DHT_EP_ENABLE( bool isEnable);
/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t AT_ZCL_DHT_GEN_CmdCallbacks =
{ 
  AT_ZCL_DHT_BasicResetCB,                 // Basic Cluster Reset command
  AT_ZCL_DHT_IdentifyCB,                   // Identify command   
  NULL,                                      // Identify Query Response command
  AT_ZCL_DHT_OnOffCB,                      // On/Off cluster command
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



void AT_ZCL_DHT_Init( byte task_id )
{
  AT_ZCL_DHT_TaskID = task_id;

  // This app is part of the Home Automation Profile
  zclHA_Init( &AT_ZCL_DHT_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( AT_ZCL_DHT_ENDPOINT, &AT_ZCL_DHT_GEN_CmdCallbacks );
  
  // Register the application's attribute list
  zcl_registerAttrList( AT_ZCL_DHT_ENDPOINT, AT_ZCL_DHT_MAX_ATTRIBUTES, AT_ZCL_DHT_Attrs );
  
  //register for AT command system enable/disable call back function
  AT_ZCL_EP_ENABLE_Register(  AT_ZCL_DHT_ENDPOINT,AT_ZCL_DHT_EP_ENABLE);
  
  //initialize the ONOFF device such as: a relay
  //no need for initialize
  
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
uint16 AT_ZCL_DHT_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( AT_ZCL_DHT_TaskID )) )
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

  if ( events & AT_ZCL_DHT_IDENTIFY_TIMEOUT_EVT )
  {
    if (AT_ZCL_DHT_IdentifyTime > 0 )
      AT_ZCL_DHT_IdentifyTime--;
    AT_ZCL_DHT_ProcessIdentifyTimeChange();

    return ( events ^ AT_ZCL_DHT_IDENTIFY_TIMEOUT_EVT );
  }
  //measure the temperature and relative humidity
  if ( events & AT_ZCL_DHT_MEASURE_EVT )
  {
    AT_ZCL_DHT_update();
    osal_start_timerEx( AT_ZCL_DHT_TaskID, AT_ZCL_DHT_MEASURE_EVT, 1000 );
    return ( events ^ AT_ZCL_DHT_MEASURE_EVT );
  }

  
  // Discard unknown events
  return 0;
}









/*********************************************************************
 * @fn      AT_ZCL_DHT_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void AT_ZCL_DHT_BasicResetCB( void )
{
  // Reset all attributes to default values
  AT_ZCL_DHT_OnOffCB( COMMAND_OFF );
  
  
  zclIdentify_t identifyCmd={NULL,0};
  AT_ZCL_DHT_IdentifyCB( &identifyCmd );
}


/*********************************************************************
 * @fn      AT_ZCL_DHT_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   srcAddr - source address and endpoint of the response message
 * @param   identifyTime - the number of seconds to identify yourself
 *
 * @return  none
 */
static void AT_ZCL_DHT_IdentifyCB( zclIdentify_t *pCmd )
{
  AT_ZCL_DHT_IdentifyTime = pCmd->identifyTime;
  AT_ZCL_DHT_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      AT_ZCL_DHT_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   none
 *
 * @return  none
 */
static void AT_ZCL_DHT_ProcessIdentifyTimeChange( void )
{
  if ( AT_ZCL_DHT_IdentifyTime > 0 )
  {
    osal_start_timerEx( AT_ZCL_DHT_TaskID, AT_ZCL_DHT_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_1, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    if ( AT_ZCL_DHT_OnOff )
      HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
    else
      HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
    osal_stop_timerEx( AT_ZCL_DHT_TaskID, AT_ZCL_DHT_IDENTIFY_TIMEOUT_EVT );
  }
}



/*********************************************************************
 * @fn      AT_ZCL_DHT_OnOffCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   cmd - COMMAND_ON, COMMAND_OFF or COMMAND_TOGGLE
 *
 * @return  none
 */
static void AT_ZCL_DHT_OnOffCB( uint8 cmd )
{
  // Turn on the light
  if ( cmd == COMMAND_ON )
    AT_ZCL_DHT_OnOff = AT_ZCL_GEN_ON;

  // Turn off the light
  else if ( cmd == COMMAND_OFF )
    AT_ZCL_DHT_OnOff = AT_ZCL_GEN_OFF;

  // Toggle the light
  else
  {
    if ( AT_ZCL_DHT_OnOff == AT_ZCL_GEN_OFF )
      AT_ZCL_DHT_OnOff = AT_ZCL_GEN_ON;
    else
      AT_ZCL_DHT_OnOff = AT_ZCL_GEN_OFF;
  }

  // In this sample app, we use LED4 to simulate the Light
  if ( AT_ZCL_DHT_OnOff == AT_ZCL_GEN_ON ){
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    osal_start_timerEx( AT_ZCL_DHT_TaskID, AT_ZCL_DHT_MEASURE_EVT, 10 );
  }
  else{
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    osal_stop_timerEx( AT_ZCL_DHT_TaskID, AT_ZCL_DHT_MEASURE_EVT );
    AT_ZCL_DHT_T_current=0xffff;   //0xfff indicates that the temperature measurement is invalid.
    AT_ZCL_DHT_RH_current=0xffff;   //0xfff indicates that the reletive humidity measurement is invalid.
  }
}


/************************************************
update relative humidity and temperature value
**************************************************/
static void AT_ZCL_DHT_update(void){
  DHT11();  //read the data from DHT
  AT_ZCL_DHT_T_current = ((uint16)ucharT_data_H)*100+ ((((uint16)ucharT_data_L)*100)>>8);
  AT_ZCL_DHT_RH_current = ((uint16)ucharRH_data_H)*100+ ((((uint16)ucharRH_data_L)*100)>>8);
}
/******************************************************
 * @fn      AT_ZCL_DHT_EP_ENABLE
 *
 * @brief   Process  Measurement and Sensing profile call Callback function
********************************************************/

static void AT_ZCL_DHT_EP_ENABLE( bool isEnable){
  if(isEnable) {
    AT_ZCL_DHT_OnOffCB(COMMAND_ON);
  }
  else {
    AT_ZCL_DHT_BasicResetCB( );
  }
}
