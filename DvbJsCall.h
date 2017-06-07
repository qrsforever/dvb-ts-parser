#ifndef _DvbJsCall__H_
#define _DvbJsCall__H_

#include "JsGroupCall.h"
#include "Hippo_api.h"

#define JSE_BUFFER_SIZE 4096

#ifdef __cplusplus
class DvbJsCall : public Hippo::JsGroupCall {
};
#endif // __cplusplus

int DvbJsCallInit(ioctl_context_type_e);

#endif // _DvbJsCall_H_
