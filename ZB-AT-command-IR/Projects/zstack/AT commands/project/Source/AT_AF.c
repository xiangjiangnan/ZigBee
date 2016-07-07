/**************************************************************************************************
  Filename:       AT_AF.c

  Description:    AT command module
  Author:         Xiao Wang
**************************************************************************************************/


#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"


#include "At_include.h"

#include "hal_led.h"
#include "hal_uart.h"
#include "zcl.h"
/*************************************global variable*****************
*
*/


aps_Group_t AT_AF_Group={
  AT_AF_GROUP_ID,                               //group id
  {'A','T','_','A','F','_','g','r','o','u','p','\0'}    //group name "AT_AF_group"
};

const cId_t AT_AF_ClusterList[AT_AF_MAX_CLUSTERS] = AT_AF_ClusterList_ ;

const SimpleDescriptionFormat_t AT_AF_SimpleDesc =
{
  AT_AF_ENDPOINT,              //  int Endpoint;
  AT_AF_PROFID,                //  uint16 AppProfId[2];
  AT_AF_DEVICEID,              //  uint16 AppDeviceId[2];
  AT_AF_DEVICE_VERSION,        //  int   AppDevVer:4;
  AT_AF_FLAGS,                 //  int   AppFlags:4;
  AT_AF_MAX_CLUSTERS,                                     //  uint8  AppNumInClusters;
  (cId_t *)AT_AF_ClusterList,                            //  uint8 *pAppInClusterList;
  AT_AF_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)AT_AF_ClusterList   //  uint8 *pAppInClusterList;
};

endPointDesc_t AT_AF_epDesc;
uint8 AT_AF_TransID;  // This is the unique message ID (counter)

/****************Local function*********************/
void AT_AF_Cmd_REPPRINT_CB(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_REPPRINT_req(afIncomingMSGPacket_t *pkt  );
void AT_AF_Cmd_REPPRINT_rsp(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_REPENABLE_CB(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_REPENABLE_req(afIncomingMSGPacket_t *pkt  );
void AT_AF_Cmd_REPENABLE_rsp(afIncomingMSGPacket_t *pkt );

void AT_AF_Cmd_HA_DISC_CB(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_HA_DISC_req(afIncomingMSGPacket_t *pkt  );
void AT_AF_Cmd_HA_DISC_rsp(afIncomingMSGPacket_t *pkt );

void AT_AF_Cmd_HA_CIDDISC_CB(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_HA_CIDDISC_req(afIncomingMSGPacket_t *pkt  );
void AT_AF_Cmd_HA_CIDDISC_rsp(afIncomingMSGPacket_t *pkt );

void AT_AF_Cmd_R_CIDDISC_CB(afIncomingMSGPacket_t *pkt );
void AT_AF_Cmd_R_CIDDISC_req(afIncomingMSGPacket_t *pkt  );
void AT_AF_Cmd_R_CIDDISC_rsp(afIncomingMSGPacket_t *pkt );

void AT_AF_Cmd_POWER_SVING_EXP_CB(afIncomingMSGPacket_t *pkt );

void AT_AF_TEST_KEY_CB(afIncomingMSGPacket_t *pkt );

void AT_AF_UPDATE_CB(afIncomingMSGPacket_t *pkt );

void AT_AF_DEV_REPORT_CB(afIncomingMSGPacket_t *pkt);

void AT_AF_Register(uint8 *task_id){
 // Fill out the endpoint description.
  AT_AF_epDesc.endPoint = AT_AF_ENDPOINT;
  AT_AF_epDesc.task_id = task_id;
  AT_AF_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&AT_AF_SimpleDesc;
  AT_AF_epDesc.latencyReq = noLatencyReqs;
  
  // Register the endpoint description with the AF
  afRegister( &AT_AF_epDesc );
  
  aps_AddGroup( AT_AF_ENDPOINT, &AT_AF_Group );
  
}


void AT_AF_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  
  switch ( pkt->clusterId )
  {
    case AT_AF_Cmd_REPPRINT_CLUSTERID:
      AT_AF_Cmd_REPPRINT_CB(pkt);
      break;
    case AT_AF_Cmd_REPENABLE_CLUSTERID:
      AT_AF_Cmd_REPENABLE_CB(pkt);
      break;
    case  AT_AF_Cmd_HA_DISC_CLUSTERID:
      AT_AF_Cmd_HA_DISC_CB(pkt);
      break;
    case  AT_AF_Cmd_HA_CIDDISC_CLUSTERID:
      AT_AF_Cmd_HA_CIDDISC_CB(pkt);
      break;
    case  AT_AF_Cmd_R_CIDDISC_CLUSTERID:
      AT_AF_Cmd_R_CIDDISC_CB(pkt);
      break;
    case  AT_AF_TEST_KEY_CLUSTERID:
      AT_AF_TEST_KEY_CB(pkt);
      break;
    case  AT_AF_POWER_SVING_EXP_CLUSTERID:
      AT_AF_Cmd_POWER_SVING_EXP_CB(pkt);
      break;
    case  AT_AF_UPDATE_CLUSTERID:
      AT_AF_UPDATE_CB(pkt);
      break;
    case  AT_AF_DEV_REPORT_CLUSTERID:
      AT_AF_DEV_REPORT_CB(pkt);
      break;
  }
}


afStatus_t AT_AF_Cmd_send_simple_(uint16 nwkAddr,uint16 CID,uint8 len, uint8 *buff){
  
  afAddrType_t dstAddr;
  dstAddr.endPoint = AT_AF_ENDPOINT;
  dstAddr.addr.shortAddr =nwkAddr;
  dstAddr.addrMode = (afAddrMode_t) Addr16Bit;
  
  return AF_DataRequest( &dstAddr, & AT_AF_epDesc,
                       CID,
                       len,
                       buff,
                       &AT_AF_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS );
}



void AT_AF_Cmd_REPPRINT_req(afIncomingMSGPacket_t *pkt ){
  const uint8 AT_CMD_EP_ARRAY[] = AT_CMD_EPs;
  uint8 epNum = afNumEndPoints()-1;
  byte* epBuf = (byte*)  osal_mem_alloc(epNum);
  if(epBuf==NULL) return;
  afEndPoints( epBuf, true);
  AT_sort_arr(epBuf,epNum);
  
  //the epStatus store the search result
  uint8 pbuf_temp[sizeof(AT_CMD_EP_ARRAY)+sizeof(AT_AF_hdr)];
  AT_AF_Cmd_REPPRINT_rsp_t* pbuf = (AT_AF_Cmd_REPPRINT_rsp_t*)pbuf_temp;
  pbuf->hdr.cmd=AT_AF_Cmd_rsp;
  pbuf->hdr.numItem=AT_CMD_EPs_Num;
  
  int i,j;
  for(i=0,j=0;j<AT_CMD_EPs_Num;j++){
    if(epBuf[i]==AT_CMD_EP_ARRAY[j]){
      pbuf->status[j] = AT_AF_enable;
      i++;
    }
    else pbuf->status[j] = AT_AF_disable;
  }
  osal_mem_free(epBuf);
  AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                       AT_AF_Cmd_REPPRINT_CLUSTERID,
                       sizeof(AT_CMD_EP_ARRAY)+sizeof(AT_AF_hdr),
                       (uint8*)pbuf,
                       &AT_AF_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS );
  
}
void AT_AF_Cmd_REPPRINT_rsp(afIncomingMSGPacket_t *pkt ){
  uint8* epStatus = ((AT_AF_Cmd_REPPRINT_rsp_t*) pkt->cmd.Data)->status;
  const uint8 AT_CMD_EP_ARRAY[] = AT_CMD_EPs;
  
  uint8 i;
  char str[4];
  AT_RESP_START();
  AT_RESP("<nodeID>,<EndPoint>:<Status>",
          sizeof("<nodeID>,<EndPoint>:<Status>")-1);
  for(i=0;i<((AT_AF_Cmd_REPPRINT_rsp_t*) pkt->cmd.Data)->hdr.numItem;i++){
    AT_NEXT_LINE();
    //display nodeID
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      AT_Int16toChar(pkt->srcAddr.addr.shortAddr,str);
      AT_RESP(str,4);
    }else AT_RESP("UNKNOWN",sizeof("UNKNOWN")-1);
    AT_RESP(",",1);
    
    //print end point
    AT_Int8toChar(AT_CMD_EP_ARRAY[i],str);
    AT_RESP(str,2);
    
    //display status 
    if(epStatus[i]==AT_AF_enable){
      AT_RESP(":ENABLED",sizeof(":ENABLED")-1);
    }
    else AT_RESP(":DISABLED",sizeof(":DISABLED")-1);
  }
  AT_RESP_END();
}


void AT_AF_Cmd_REPPRINT_CB(afIncomingMSGPacket_t *pkt ){
  
  AT_AF_hdr *hdr = (AT_AF_hdr*)pkt->cmd.Data;
  switch (hdr->cmd){
  case AT_AF_Cmd_req:
    AT_AF_Cmd_REPPRINT_req(pkt);
    break;
  case AT_AF_Cmd_rsp:
    AT_AF_Cmd_REPPRINT_rsp(pkt);
    break;
  }   
}


/*************************************************************
    deal with the requst from other devices for REPENABLE, 
    and send a response
*************************************************************/
void AT_AF_Cmd_REPENABLE_req(afIncomingMSGPacket_t *pkt  ){
  AT_AF_Cmd_REPENABLE_rsp_t rsp;
  
  AT_AF_Cmd_REPENABLE_req_t *req = (AT_AF_Cmd_REPENABLE_req_t*)pkt->cmd.Data;
  uint8 enable =req->enable;
  uint8 ep     =req->ep;
  
  rsp.hdr.cmd=AT_AF_Cmd_rsp;
  rsp.item.ep=req->ep;
  
  if(enable){
    if( AT_af_get_ep(ep)){
      if(AT_af_register_ep(ep)==afStatus_SUCCESS){ 
        //enable the end point in ZCL layer
        AT_ZCL_EP_ENABLE( enable,ep);
        AT_NV_ZCL_saveEPStatus(AT_NV_ZCL_get_index_(ep),&enable);
      }
      rsp.item.status=AT_AF_enable;
    }else {
      rsp.item.status = AT_AF_UNKNOWNEP;
    }
  }
  else{

    if(AT_af_get_ep(ep)){
      if(AT_af_remove_ep(ep)==afStatus_SUCCESS){
        //disable the end point in ZCL layer
        AT_ZCL_EP_ENABLE( enable,ep);
        //save to NV
        AT_NV_ZCL_saveEPStatus(AT_NV_ZCL_get_index_(ep),&enable);
      }
      rsp.item.status = AT_AF_disable;
    }else {
      rsp.item.status = AT_AF_UNKNOWNEP;
    }
  }
  
  AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                       AT_AF_Cmd_REPENABLE_CLUSTERID,
                       sizeof(AT_AF_Cmd_REPENABLE_rsp_t),
                       (uint8*)&rsp,
                       &AT_AF_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS );
  
}

/*************************************************************
deal with the response from other devices
*************************************************************/
void AT_AF_Cmd_REPENABLE_rsp(afIncomingMSGPacket_t *pkt ){
  AT_AF_Cmd_REPENABLE_rsp_t *rsp = (AT_AF_Cmd_REPENABLE_rsp_t*)pkt->cmd.Data;
  uint8 status =rsp->item.status;
  uint8 ep     =rsp->item.ep;
  char str[4];
  AT_RESP_START();
       
  switch (status){
  case AT_AF_enable:
    AT_RESP("ENABLED:",sizeof("ENABLED:")-1);
    //print nodeID
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      AT_Int16toChar(pkt->srcAddr.addr.shortAddr,str);
      AT_RESP(str,4);
    }else AT_RESP("UNKNOWN",sizeof("UNKNOWN")-1);
    AT_RESP(",",1); 
    //print end point
    AT_Int8toChar(ep,str);
    AT_RESP(str,2);
    break;
  case AT_AF_disable:
    AT_RESP("DISABLED:",sizeof("DISABLED:")-1);
    //print nodeID
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      AT_Int16toChar(pkt->srcAddr.addr.shortAddr,str);
      AT_RESP(str,4);
    }else AT_RESP("UNKNOWN",sizeof("UNKNOWN")-1);
    AT_RESP(",",1); 
    //print end point
    AT_Int8toChar(ep,str);
    AT_RESP(str,2);
    break;
  case AT_AF_UNKNOWNEP:
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      AT_Int16toChar(pkt->srcAddr.addr.shortAddr,str);
      AT_RESP(str,4);
    }else AT_RESP("UNKNOWN",sizeof("UNKNOWN")-1);
    AT_RESP(",",1); 
    //print end point
    AT_RESP("UNKNOWNEP",sizeof("UNKNOWNEP")-1);
    break;
  }
  AT_RESP_END();
}
/**************************************************************
deal with AT_AF_Cmd_REPENABLE_CLUSTERID messages.
*************************************************************/
void AT_AF_Cmd_REPENABLE_CB(afIncomingMSGPacket_t *pkt ){
  AT_AF_Cmd_REPENABLE_req_t *req = (AT_AF_Cmd_REPENABLE_req_t *)pkt->cmd.Data;
  if(req->hdr.cmd==AT_AF_Cmd_req){
    AT_AF_Cmd_REPENABLE_req(pkt);
  }else{
    AT_AF_Cmd_REPENABLE_rsp(pkt);
  }
}



void AT_AF_Cmd_HA_DISC_CB(afIncomingMSGPacket_t *pkt ){
  AT_AF_hdr *req = (AT_AF_hdr *)pkt->cmd.Data;
  if(req->cmd==AT_AF_Cmd_req){
    AT_AF_Cmd_HA_DISC_req(pkt);
  }else{
    AT_AF_Cmd_HA_DISC_rsp(pkt);
  }
}

/***************************************************************************
deal with the HA discover request
*****************************************************************************/
void AT_AF_Cmd_HA_DISC_req(afIncomingMSGPacket_t *pkt  ){
  AT_AF_Cmd_HA_DISC_req_t *req = (AT_AF_Cmd_HA_DISC_req_t *)pkt->cmd.Data;
  AT_AF_Cmd_HA_DISC_rsp_t *rsp;
  uint8 zcl_epCount =0;
  
  uint8 AT_CMD_EP_ARRAY[]=AT_CMD_EPs;
  uint8 i;
  uint8 buf_temp[sizeof(AT_AF_Cmd_HA_DISC_rsp_t)+sizeof(AT_CMD_EP_ARRAY)*sizeof(AT_AF_Cmd_HA_DISC_item_t)];
  rsp = (AT_AF_Cmd_HA_DISC_rsp_t *) buf_temp;
  
  for(i=0;i<sizeof(AT_CMD_EP_ARRAY);i++){
    epList_t* epList = AT_af_get_ep(AT_CMD_EP_ARRAY[i]);
    SimpleDescriptionFormat_t* simDesc=epList->epDesc->simpleDesc;
    
    //find the CID
    bool found=false;
    uint8 j;
    for(j=0;j<simDesc->AppNumOutClusters;j++){
      if(simDesc->pAppOutClusterList[j]==req->CID) {
        if(req->option==0){
          if(afFindEndPointDesc(AT_CMD_EP_ARRAY[i])) found=true;
        }else{
          found=true;
        }
        break;
      }
    }
    if(!found){
      for(j=0;j<simDesc->AppNumInClusters;j++){
        if(simDesc->pAppInClusterList[j]==req->CID) {
          if(req->option==0){
            if(afFindEndPointDesc(AT_CMD_EP_ARRAY[i])) found=true;
          }else{
            found=true;
          }
          break;
        }
      }
    }
    
    //if found the CID
    if(found){
      rsp->item[zcl_epCount].ep = AT_CMD_EP_ARRAY[i];
      rsp->item[zcl_epCount].status = afFindEndPointDesc(AT_CMD_EP_ARRAY[i]) ? AT_AF_enable : AT_AF_disable;
      zcl_epCount++;
    }
  }
  if(zcl_epCount!=0){
    rsp->hdr.numItem=zcl_epCount;
    rsp->hdr.cmd=AT_AF_Cmd_rsp;
    osal_memcpy( rsp->MAC, NLME_GetExtAddr(), 8);//fill in MAC addr NLME_GetExtAddr()
    AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                         AT_AF_Cmd_HA_DISC_CLUSTERID,
                         sizeof(AT_AF_Cmd_HA_DISC_rsp_t)+zcl_epCount*sizeof(AT_AF_Cmd_HA_DISC_item_t),
                         (uint8*)rsp,
                         &AT_AF_TransID,
                         AF_DISCV_ROUTE,
                         AF_DEFAULT_RADIUS );
      
  }
}
/***************************************************************************************
+DISCOVER �C Discover HA Devices On The HAN
Prompt: 
    DEV:<NodeID>,<EndPoint> ,<enable/disable>
    Carry out the ZigBee Service Discovery to 
    find ZigBee HA devices that support the given match criteria.

****************************************************************************************/
void AT_AF_Cmd_HA_DISC_rsp(afIncomingMSGPacket_t *pkt ){
  AT_AF_Cmd_HA_DISC_rsp_t *rsp = (AT_AF_Cmd_HA_DISC_rsp_t *)pkt->cmd.Data;
  AT_RESP_START();
  uint8 i;
  
  //change MAC address to char 
  char str[17];
  AT_EUI64toChar(rsp->MAC,str);
  str[sizeof(str)-1]='\0';
  
  for(i=0;i<rsp->hdr.numItem;i++){
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      printf("DEV:%s,%04X,",str,pkt->srcAddr.addr.shortAddr);
    }else {
      printf("UNKNOWN");
    }
    printf("%02X,",rsp->item[i].ep);       
    if(rsp->item[i].status==AT_AF_enable){
      printf("ENABLED");
    }
    else printf("DISABLED");
    
    AT_NEXT_LINE();
  }
  AT_RESP_START();
}
/********************************************************
call back funtin of the CLUSDISC command

DISCCLUS:<Node ID>,<EndPoint>,<Status>
Prompt (will show, if <Status> is 00) 
            SERVER:<ClusterID>,<ClusterID>,<ClusterID> 
            CLIENT:<ClusterID>,<ClusterID>,< ClusterI
*************************************************************/
void AT_AF_Cmd_HA_CIDDISC_CB(afIncomingMSGPacket_t *pkt ){
  AT_AF_hdr *req = (AT_AF_hdr *)pkt->cmd.Data;
  if(req->cmd==AT_AF_Cmd_req){
    AT_AF_Cmd_HA_CIDDISC_req(pkt);
  }else{
    AT_AF_Cmd_HA_CIDDISC_rsp(pkt);
  }
}
/********************************************************
process request funtin of the CLUSDISC command
*************************************************************/
void AT_AF_Cmd_HA_CIDDISC_req(afIncomingMSGPacket_t *pkt  ){
    AT_AF_hdr *req = (AT_AF_hdr *)pkt->cmd.Data;
    AT_AF_Cmd_HA_CIDDISC_rsp_t *rsp;
    uint8 status=0;
    uint16 rspLen=0;
    if(AT_is_CMD_EPs(req->ep)){
      SimpleDescriptionFormat_t *pDesc;
      if(SUCCESS == (status=afFindSimpleDesc(&pDesc, req->ep))){
        rspLen = (pDesc->AppNumInClusters+pDesc->AppNumInClusters)*2 +sizeof(AT_AF_Cmd_HA_CIDDISC_rsp_t);
        rsp = osal_mem_alloc(rspLen);
        if(rsp) {
          rsp->status = SUCCESS;
          rsp->serverNum = pDesc->AppNumInClusters;
          rsp->clientNum = pDesc->AppNumOutClusters;
          rsp->hdr.ep = req->ep;
          rsp->hdr.cmd = AT_AF_Cmd_rsp;
          osal_memcpy(rsp->list,pDesc->pAppInClusterList,rsp->serverNum*2);
          osal_memcpy(rsp->list+rsp->serverNum,pDesc->pAppOutClusterList,rsp->clientNum*2);
          //sent the found data
          AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                               AT_AF_Cmd_HA_CIDDISC_CLUSTERID,
                               rspLen,
                               (uint8*)rsp,
                               &AT_AF_TransID,
                               AF_DISCV_ROUTE,
                               AF_DEFAULT_RADIUS );
          osal_mem_free(rsp);
          return;     //important
        }else{
          status = AT_AF_MEM_ERROR;
        }
      }else {
        //status = status; //have been done
      }
    }else{
      status = AT_AF_NOTFOUND_ERROR;
    }
    
    AT_AF_Cmd_HA_CIDDISC_rsp_t dftRsp;
    dftRsp.hdr.cmd = AT_AF_Cmd_rsp;
    dftRsp.hdr.ep = req->ep;
    dftRsp.status = status;
    rsp = &dftRsp;
    AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                         AT_AF_Cmd_HA_CIDDISC_CLUSTERID,
                         sizeof(AT_AF_Cmd_HA_CIDDISC_rsp_t),
                         (uint8*)rsp,
                         &AT_AF_TransID,
                         AF_DISCV_ROUTE,
                         AF_DEFAULT_RADIUS );
}

/********************************************************
process response funtin of the CLUSDISC command

DISCCLUS:<Node ID>,<EndPoint>,<Status>
Prompt (will show, if <Status> is 00) 
            SERVER:<ClusterID>,<ClusterID>,<ClusterID> 
            CLIENT:<ClusterID>,<ClusterID>,< ClusterI
*************************************************************/
void AT_AF_Cmd_HA_CIDDISC_rsp(afIncomingMSGPacket_t *pkt ){
    AT_AF_Cmd_HA_CIDDISC_rsp_t *rsp = (AT_AF_Cmd_HA_CIDDISC_rsp_t *) pkt->cmd.Data;
    AT_RESP_START();
    AT_DEBUG("DISCCLUS:<Node ID>,<EndPoint>,<Status>",sizeof("       <Node ID>,<EndPoint>,<Status>"));
    AT_NEXT_LINE();
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      printf("DISCCLUS:%04X",pkt->srcAddr.addr.shortAddr);
    }else printf("DISCCLUS:UNKNOWN");
    
    printf(",%02X,%02X",rsp->hdr.ep,rsp->status);
    
    if(rsp->status==SUCCESS){
      AT_NEXT_LINE();
      printf("SERVER:");
      uint8 i;
      for(i=0;i<rsp->serverNum;i++) printf(",%04X",rsp->list[i]);
      AT_NEXT_LINE();
      printf("CLIENT:");
      for(i=0;i<rsp->clientNum;i++) printf(",%04X",rsp->list[i+rsp->serverNum]);
    }
    AT_RESP_END();
}


/*******************************************************************
for AT+R command
**************************************************************************/

void AT_AF_Cmd_R_CIDDISC_CB(afIncomingMSGPacket_t *pkt ){
  AT_AF_hdr *req = (AT_AF_hdr *)pkt->cmd.Data;
  if(req->cmd==AT_AF_Cmd_req){
    AT_AF_Cmd_R_CIDDISC_req(pkt);
  }else{
    AT_AF_Cmd_R_CIDDISC_rsp(pkt);
  }
}
void AT_AF_Cmd_R_CIDDISC_req(afIncomingMSGPacket_t *pkt  ){
  
  AT_AF_Cmd_RCIDDISC_req_t *req = (AT_AF_Cmd_RCIDDISC_req_t *) pkt->cmd.Data;
  AT_HandleCMD(req->list);
}

void AT_AF_Cmd_R_CIDDISC_rsp(afIncomingMSGPacket_t *pkt ){
}

/**********************************************************************
flashing the led_2 to indication which device in the PAN
***********************************************************************/
void AT_AF_TEST_KEY_CB(afIncomingMSGPacket_t *pkt ){
  HalLedBlink( HAL_LED_2, 4, 50, 250 );
}


/**************************************************************************
Power saving experiment for the Papper of power efficiency in ZigBee
******************************************************************************/
static void AT_AF_Cmd_RSSI_req(afIncomingMSGPacket_t *pkt  );
static void AT_AF_Cmd_RSSI_rsp(afIncomingMSGPacket_t *pkt );
static void AT_AF_Cmd_PSE_req(afIncomingMSGPacket_t *pkt);

static void AT_AF_Cmd_RSSI_req(afIncomingMSGPacket_t *pkt  ){
  AT_AF_Cmd_POWER_SAVING_rssi_t buf;
  buf.hdr.cmd =AT_AT_PSE_RSSI_rsp;
  buf.rssi = pkt->rssi;
  buf.lqi = pkt->LinkQuality;
  buf.correlation =pkt->correlation;
  AF_DataRequest( & (pkt->srcAddr), & AT_AF_epDesc,
                         AT_AF_POWER_SVING_EXP_CLUSTERID,
                         sizeof(buf),
                         (uint8*)&buf,
                         &AT_AF_TransID,
                         AF_DISCV_ROUTE,
                         AF_DEFAULT_RADIUS );
}

static void AT_AF_Cmd_RSSI_rsp(afIncomingMSGPacket_t *pkt ){
  AT_AF_Cmd_POWER_SAVING_rssi_t* rsp = (AT_AF_Cmd_POWER_SAVING_rssi_t*)pkt->cmd.Data;
  AT_RESP_START();
  printf("RSSIRESP:LQI_send=%d,RSSI_send=%d,correlation_send=%d",
         rsp->lqi,
         rsp->rssi,
         rsp->correlation);
  AT_NEXT_LINE();
  printf("RSSIRESP:LQI_receive=%d,RSSI_receive=%d,correlation_receive=%d",
         pkt->LinkQuality,
         pkt->rssi,
         pkt->correlation);
  AT_RESP_END();
}
static void AT_AF_Cmd_PSE_req(afIncomingMSGPacket_t *pkt){
  AT_AF_Cmd_POWER_SAVING_EXP_t *req = (AT_AF_Cmd_POWER_SAVING_EXP_t *)pkt->cmd.Data;
  static uint16 package_count=0;
  switch(req->hdr.info){
  case AT_AF_PSE_info_pre:
    package_count=0;
    AT_RESP_START();
    printf("%d packages are coming with interval %dms...",
           req->count,
           req->interval);
    AT_RESP_END();
    
    printf("Package Received:");
    AT_NEXT_LINE();
    break;
  case AT_AF_PSE_info_start:
    break;
  case AT_AF_PSE_info_ing:
    package_count++;
    
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_TOGGLE );
    printf("\r%d",package_count);
    break;
  case AT_AF_PSE_info_end:
    printf("\r%d",package_count);
    
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
    AT_NEXT_LINE();
    printf("Receive finished!");
    AT_NEXT_LINE();
    break;
  }
}

void AT_AF_Cmd_POWER_SVING_EXP_CB(afIncomingMSGPacket_t *pkt ){
   AT_AF_hdr *req = (AT_AF_hdr *)pkt->cmd.Data;
  switch (req->cmd){
  case AT_AT_PSE_RSSI_req:
    AT_AF_Cmd_RSSI_req(pkt);
    break;
  case AT_AT_PSE_RSSI_rsp:
    AT_AF_Cmd_RSSI_rsp(pkt);
    break;
  case AT_AT_PSE_EXP_req:
    AT_AF_Cmd_PSE_req(pkt);
    break;
  }
}

/***************************************************************
* API for sending update message!!
*/
afStatus_t AT_AF_send_update(uint8 ep,uint16 clusterId,uint16 attrID,uint8 dataType, uint8* data,uint8 status){
  AT_AF_UPDATE_t *buf; 
  byte* epBuf;
  uint8 size;
  if(dataType == ZCL_DATATYPE_CHAR_STR){
    epBuf = (byte*)  osal_mem_alloc(size=(sizeof(AT_AF_UPDATE_t)+data[0]+1));  //alloc memory for string type
  }else if      (dataType == ZCL_DATATYPE_DATA16||
                 dataType == ZCL_DATATYPE_UINT16||
                 dataType == ZCL_DATATYPE_INT16){
    epBuf = (byte*)  osal_mem_alloc(size=(sizeof(AT_AF_UPDATE_t)+2));  //alloc memory for int16 or unint16 or data16
  }else{
    epBuf = (byte*)  osal_mem_alloc(size=(sizeof(AT_AF_UPDATE_t)+1));  //WARNINIG::::alloc memory for other, here is a little bug, but its OK for current usage
  }
  if(!epBuf) return ZMemError;          //inidate the memory alloc failure
  buf=  (AT_AF_UPDATE_t *) epBuf;     
  osal_memcpy( buf->MAC, NLME_GetExtAddr(), 8);//fill in MAC addr NLME_GetExtAddr()
  buf->ep = ep;
  buf->status=status;
  buf->clusterId=clusterId;
  buf->attrID=attrID;
  buf->dataType=dataType;
  osal_memcpy( buf->data, data, size - sizeof(AT_AF_UPDATE_t));//fill in data
  uint8 afstatus = AT_AF_Cmd_send_simple(0,AT_AF_UPDATE_CLUSTERID,size, buf);   //send the message the COOR
  
  osal_mem_free(epBuf);         //free memory
  return afstatus;
}

void AT_AF_UPDATE_CB(afIncomingMSGPacket_t *pkt ){
  AT_AF_UPDATE_t *dat = (AT_AF_UPDATE_t*)pkt->cmd.Data;
  AT_RESP_START();
  if(dat->status==0){
    //change MAC address to char 
    char str[17];
    AT_EUI64toChar(dat->MAC,str);
    str[sizeof(str)-1]='\0';
    
    //UPDATE:<MAC>,<NWK>,<EP>,<ClusterID>,<AttrID>,<DataType>,<data>
    
    //pirnt UPDATE:<MAC>,<NWK>,
    if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
      printf("UPDATE:%s,%04X,",str,pkt->srcAddr.addr.shortAddr);
    }else {
      printf("UPDATE:%s,UNKNOWN NWK,",str);
    }
    //pirnt <EP>,<ClusterID>,<AttrID>,<DataType>,
    printf("%02X,%04X,%04X,%02X,",dat->ep,
                                  dat->clusterId,
                                  dat-> attrID,
                                  dat->dataType);
    
    //print <data>
    if(dat->dataType == ZCL_DATATYPE_CHAR_STR){
          AT_RESP(dat->data +1,dat->data[0]);        
        }else if(dat->dataType == ZCL_DATATYPE_DATA16||
                 dat->dataType == ZCL_DATATYPE_UINT16||
                 dat->dataType == ZCL_DATATYPE_INT16){
          AT_Int16toChar(*((uint16*)dat->data),str);
          AT_RESP(str,4);
        }else{
          
          AT_Int8toChar((uint8)dat->data[0],str);
          AT_RESP(str,2);
        }
  }else{
      printf("UPDATE: status error");
  }
  AT_RESP_END();
}

/***************************************************************
* API for sending DEV report message!!
*/
afStatus_t AT_AF_send_DEV_REPORT(){
  uint8 eps[] = AT_CMD_EPs;
  uint8 buff[sizeof(AT_AF_DEV_REPORT_t)+sizeof(eps)];
  AT_AF_DEV_REPORT_t* buf = (AT_AF_DEV_REPORT_t*)&buff;
  osal_memcpy( buf->MAC, NLME_GetExtAddr(), 8);//fill in MAC addr NLME_GetExtAddr()
  buf->itemNum=sizeof(eps);
  osal_memcpy( buf->epList, eps, sizeof(eps));
  return AT_AF_Cmd_send_simple(0,AT_AF_DEV_REPORT_CLUSTERID,sizeof(AT_AF_DEV_REPORT_t)+sizeof(eps), buf);
}

void AT_AF_DEV_REPORT_CB(afIncomingMSGPacket_t *pkt){
  AT_AF_DEV_REPORT_t *dat = (AT_AF_DEV_REPORT_t*)pkt->cmd.Data;
  AT_RESP_START();
  //change MAC address to char 
  char str[17];
  AT_EUI64toChar(dat->MAC,str);
  str[sizeof(str)-1]='\0';
  
  //DEVRPT:<MAC>,<NWK>,<EP1>,<EP2>...
  
  //print DEVRPT:<MAC>,NWK
  if(pkt->srcAddr.addrMode==(afAddrMode_t)Addr16Bit){
    printf("DEVRPT:%s,%04X",str,pkt->srcAddr.addr.shortAddr);
  }else {
    printf("DEVRPT:%s,UNKNOWN NWK,",str);
  }
  //pirnt ,<EP1>,<EP2>...
  uint8 i=0;
  for(;i<dat->itemNum;i++){
    printf(",%02X",dat->epList[i]);
  }
  AT_RESP_END();
}
