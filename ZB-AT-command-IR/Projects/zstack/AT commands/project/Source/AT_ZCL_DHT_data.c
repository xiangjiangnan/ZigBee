/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ms.h"
#include "zcl_ha.h"

#include "AT_ZCL_DHT.h"


#define AT_ZCL_DHT_DEVICE_VERSION     0
#define AT_ZCL_DHT_FLAGS              0

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Basic Cluster
const uint8 AT_ZCL_DHT_ManufacturerName[] = { 5, 'E','S','S','E','X' };
const uint8 AT_ZCL_DHT_ModelId[] = { 5, 'D','H','T','1','1' };
const uint8 AT_ZCL_DHT_DateCode[] = { 8, '2','0','1','5','0','4','1','8'};
const uint8 AT_ZCL_DHT_PowerSource = POWER_SOURCE_BATTERY;
uint8 AT_ZCL_DHT_LocationDescription[]={ 10, 'L','A','B','0','0','7',' ',' ',' ',' ' };
uint8 AT_ZCL_DHT_DeviceEnable = DEVICE_ENABLED;
//Tempetrue
uint16 AT_ZCL_DHT_T_current =0xffff;
const uint16 AT_ZCL_DTH_T_max_value = 5000;  //stand for 50 C
const uint16 AT_ZCL_DTH_T_min_value = 0;    //stand for 0 C
//Relative Humidity
uint16 AT_ZCL_DHT_RH_current =0xffff;
const uint16 AT_ZCL_DTH_RH_max_value = 9000; //stand for 90% RH
const uint16 AT_ZCL_DTH_RH_min_value = 2000;  //stand for 20% RH

// Device Temperature Configuration Cluster
int16 AT_ZCL_DHT_current=0;

// Identify Cluster
uint16 AT_ZCL_DHT_IdentifyTime = 0;


// On/Off Cluster
uint8  AT_ZCL_DHT_OnOff = AT_ZCL_GEN_OFF;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t AT_ZCL_DHT_Attrs[AT_ZCL_DHT_MAX_ATTRIBUTES] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_CHAR_STR,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)AT_ZCL_DHT_ManufacturerName  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)AT_ZCL_DHT_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)AT_ZCL_DHT_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DHT_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)AT_ZCL_DHT_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&AT_ZCL_DHT_DeviceEnable
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&AT_ZCL_DHT_IdentifyTime
    }
  },
  
  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DHT_OnOff
    }
  },
  
   
  // *** Temperature Measurement Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
    { // Attribute record
      ATTRID_MS_TEMPERATURE_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DHT_T_current
    }
  },
  {
    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
    { // Attribute record
      ATTRID_MS_TEMPERATURE_MIN_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DTH_T_min_value
    }
  },
  
  {
    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
    { // Attribute record
      ATTRID_MS_TEMPERATURE_MAX_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DTH_T_max_value
    }
  },  
  
  // *** Relative Humidity Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
    { // Attribute record
      ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DHT_RH_current
    }
  },
  {
    ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
    { // Attribute record
      ATTRID_MS_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DTH_RH_min_value
    }
  },
  
  {
    ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
    { // Attribute record
      ATTRID_MS_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ,
      (void *)&AT_ZCL_DTH_RH_max_value
    }
  },
};



/*********************************************************************
 * SIMPLE DESCRIPTOR
 */

#define AT_ZCL_DHT_MAX_INCLUSTERS       5
const cId_t AT_ZCL_DHT_InClusterList[AT_ZCL_DHT_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
  ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY
};
/*
#define AT_ZCL_DHT_MAX_OUTCLUSTERS       1
const cId_t AT_ZCL_DHT_OutClusterList[AT_ZCL_DHT_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC
};*/

SimpleDescriptionFormat_t AT_ZCL_DHT_SimpleDesc =
{
  AT_ZCL_DHT_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_TEST_DEVICE,           //  uint16 AppDeviceId[2];
  AT_ZCL_DHT_DEVICE_VERSION,            //  int   AppDevVer:4;
  AT_ZCL_DHT_FLAGS,                     //  int   AppFlags:4;
  AT_ZCL_DHT_MAX_INCLUSTERS,            //  byte  AppNumInClusters;
  (cId_t *)AT_ZCL_DHT_InClusterList,    //  byte *pAppInClusterList;
  0,                                       //  byte  AppNumInClusters;
  (cId_t *)NULL                           //  byte *pAppInClusterList;
};