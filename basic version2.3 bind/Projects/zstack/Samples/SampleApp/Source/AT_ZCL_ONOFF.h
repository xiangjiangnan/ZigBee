
#ifndef AT_ZCL_ONOFF_H
#define AT_ZCL_ONOFF_H

#include "zcl.h"


#define AT_ZCL_ONOFF_ENDPOINT            1
#define AT_ZCL_ONOFF_MAX_ATTRIBUTES      10

#define AT_ZCL_GEN_OFF                       0x00
#define AT_ZCL_GEN_ON                        0x01

#define AT_ZCL_ONOFF_IDENTIFY_TIMEOUT_EVT    0x0001
/*********************************************************************
 * Global variable
 */

extern SimpleDescriptionFormat_t AT_ZCL_ONOFF_SimpleDesc;
extern CONST zclAttrRec_t AT_ZCL_ONOFF_Attrs[];
extern uint16 AT_ZCL_ONOFF_IdentifyTime;
extern uint8  AT_ZCL_ONOFF_OnOff;


/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void AT_ZCL_ONOFF_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 AT_ZCL_ONOFF_event_loop( byte task_id, UINT16 events );

extern void AT_ZCL_ONOFF_EP_ENABLE( bool isEnable);

#endif