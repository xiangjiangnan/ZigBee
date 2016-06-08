#include "AF.h"
// These constants are only for example and should be changed to the
// device's needs
#define SAMPLEAPP_ENDPOINT           20

#define SAMPLEAPP_PROFID             0x0F08
#define SAMPLEAPP_DEVICEID           0x0001
#define SAMPLEAPP_DEVICE_VERSION     0
#define SAMPLEAPP_FLAGS              0

#define SAMPLEAPP_MAX_CLUSTERS       1
#define SAMPLEAPP_P2P_CLUSTERID      1

extern endPointDesc_t SampleApp_epDesc;
extern void AT_AF_init(void);
extern void SampleApp_Send_P2P_Message( void );