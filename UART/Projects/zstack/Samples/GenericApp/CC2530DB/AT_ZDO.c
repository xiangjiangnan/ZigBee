#include "AT_ZDO.h"
#include "ZDObject.h"
#include "ZDProfile.h"

void AT_ZDO_ProcessEDbindRsp(zdoIncomingMsg_t *inMsg );

void AT_ZDO_Register(uint8 *pTask_id){
  uint8 task_id = *pTask_id;
  ZDO_RegisterForZDOMsg( task_id,  End_Device_Bind_rsp );
}

void AT_ZDO_ProcessMsgCBs( zdoIncomingMsg_t *inMsg ){
  switch ( inMsg->clusterID )
  {
    case NWK_addr_rsp:
    case IEEE_addr_rsp:
      {
        ZDO_NwkIEEEAddrResp_t *pAddrRsp;
        pAddrRsp = ZDO_ParseAddrRsp( inMsg );
        if ( pAddrRsp )
        {
          if ( pAddrRsp->status == ZSuccess )
          {
            uint8 i;
            char str[16];
            AT_RESP_START();
            
            AT_Int16toChar(pAddrRsp->nwkAddr,str);
            AT_RESP(str,4);
            AT_RESP(",",1);
            AT_EUI64toChar(pAddrRsp->extAddr,str);
            AT_RESP(str,16);
            for( i=0;i<pAddrRsp->numAssocDevs;i++){
              AT_NEXT_LINE();
              AT_Int16toChar(pAddrRsp->devList[i],str);
              AT_RESP(str,4);
            }
            AT_RESP_END();
          }
          osal_mem_free( pAddrRsp );
        }
      }
      break;
    case End_Device_Bind_rsp:
      AT_ZDO_ProcessEDbindRsp(inMsg );
      break;
    default:
      break;
  }
}

void AT_ZDO_ProcessEDbindRsp(zdoIncomingMsg_t *inMsg ){
  uint8 status = ZDO_ParseBindRsp(inMsg);
  AT_RESP_START();
  if(inMsg->srcAddr.addrMode== (afAddrMode_t)Addr16Bit){
    printf("Ebind:%04X,%02X",inMsg->srcAddr.addr.shortAddr, status);
  }else{
    printf("Ebind:UNKNOWN,%02X", status);
  }
  AT_RESP_END();
    
}