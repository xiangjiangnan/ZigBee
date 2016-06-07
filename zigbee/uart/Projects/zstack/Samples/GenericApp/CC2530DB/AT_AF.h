#include "AF.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define GENERICAPP_ENDPOINT           10

#define GENERICAPP_PROFID             0x0104
#define GENERICAPP_DEVICEID           0x0001
#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0

#define GENERICAPP_MAX_CLUSTERS       1
#define GENERICAPP_CLUSTERID          1

//extern endPointDesc_t GenericApp_epDesc;
extern void AT_AF_init(void);
//extern void AT_AF_init(byte task_id);
extern void GenericApp_SendTheMessage(byte RxBuf[]);