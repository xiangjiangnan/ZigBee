#include "AF.h"
#include "AT_AF.h"
#include "AT_UART.h"
#include "hal_uart.h"
#include "GenericApp.h"

byte GenericApp_TransID = 0;  // This is the unique message ID (counter)
/*********************************************************************
 * GLOBAL VARIABLES
 */
// This list should be filled with Application specific Cluster IDs.
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in GenericApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t GenericApp_epDesc;

void AT_AF_init(void){
// Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  //GenericApp_epDesc.task_id = &task_id;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );
}

GenericAF_MessageMSGCB(afIncomingMSGPacket_t *pkt){
  switch ( pkt->clusterId ){
  case GENERICAPP_SEND_CLUSTERID:
    GenericApp_ReqTheMessage(pkt);
    break;
  case GENERICAPP_RECV_CLUSTERID:
    GenericApp_RespTheMessage();
    break;
  }

}
/*********************************************************************
 * @fn      GenericApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void GenericApp_SendTheMessage(char dat)
{
  NODE_INFO info;
  info.addr=NLME_GetShortAddr();
  info.MAC=NLME_GetExtAddr();
  info.DesMAC=NLME_GetCoordExtAddr();
  info.ep=10;
  info.dstep=5;
  info.CID=1;
  char cid;
  afAddrType_t P2P_DstAddr;
  P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  P2P_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  P2P_DstAddr.addr.shortAddr = 0x0000; 
 if(RxBuf[0]=='a')cid=GENERICAPP_SEND_CLUSTERID;
 
 if(RxBuf[0]=='b')cid=GENERICAPP_RECV_CLUSTERID;
  afStatus_t flag = AF_DataRequest( &P2P_DstAddr, &GenericApp_epDesc,
                       cid,
                       sizeof(NODE_INFO),
                       &info,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
 
}

/*********************************************************************
*********************************************************************/
void GenericApp_ReqTheMessage(afIncomingMSGPacket_t *pkt){
  NODE_INFO info = (NODE_INFO*)(pkt->cmd.Data);
   zAddrType_t dstAddr;
  zAddrType_t addr;
  addr.addr.shortAddr =info.addr;            
  addr.addrMode = (afAddrMode_t)Addr16Bit;
  state = ZDP_BindReq( &addr, info.MAC, info.ep,info.CID, info.DesMAC, info.dstep,0 );
      if(state==afStatus_SUCCESS){
        char Buf[]="start to bind.";
        HalUARTWrite(HAL_UART_PORT_0,Buf,len);
      }
}

void GenericApp_RespTheMessage(afIncomingMSGPacket_t *pkt){
  NODE_INFO info = (NODE_INFO*)(pkt->cmd.Data);
  zAddrType_t dstAddr;
  zAddrType_t addr;
  addr.addr.shortAddr =info.addr;            
  addr.addrMode = (afAddrMode_t)Addr16Bit;
  state = ZDP_UnbindReq( &addr, info.MAC, info.ep,info.CID, info.DesMAC, info.dstep,0 );
      if(state==afStatus_SUCCESS){
        char Buf[]="start to unbind.";
        HalUARTWrite(HAL_UART_PORT_0,Buf,len);
      }
}