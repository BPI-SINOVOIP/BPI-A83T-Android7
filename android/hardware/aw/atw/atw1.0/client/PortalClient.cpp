#include "PortalClient.h"
#include "PortalClientLocal.h"
#include "AtwLog.h"

using namespace android;

namespace allwinner
{
static PortalClient* instance = NULL;

PortalClient* PortalFactory::GetInstance()
{
    if(!instance)
    {
        instance = new PortalClientLocal();
    }
    return instance;
}

void PortalFactory::DestroyInstance(PortalClient *inst)
{
    if(inst != instance)
    {
        _LOGV("unknown client object ptr(%p), expected(%p)", inst, instance);
        return;
    }
    delete instance;
    instance = NULL;
}

};//namespace allwinner

int PortalInit(ANativeWindow** nativeSur)
{
    allwinner::PortalClient* client = allwinner::PortalFactory::GetInstance();
    if(client == NULL)
    {
       _LOGE("call %s get portal client return NULL", __func__);
       return -1;
    }

    if( 0 != client->init(nativeSur))
    {
        _LOGE("init client failed.");
        allwinner::PortalFactory::DestroyInstance(client);
        return -1;
    }
    _LOGV("init client success. return nativeSur=%p", *nativeSur);
    return 0;
}

int PortalSetTracker(PTRFUN_HeadTracker cb, int run)
{
    allwinner::PortalClient* client = allwinner::PortalFactory::GetInstance();
    if(client == NULL)
    {
       _LOGE("call %s get portal client return NULL", __func__);
       return -1;
    }

    client->setTracker(cb, run);
    return 0;
}

int PortalDestroy()
{
    allwinner::PortalClient* client = allwinner::PortalFactory::GetInstance();
    if(client == NULL)
    {
       _LOGE("call %s get portal client return NULL", __func__);
       return -1;
    }

    if(client != NULL)
    {
        allwinner::PortalFactory::DestroyInstance(client);
    }
    return 0;
}

int PortalSwap(const avrQuatf &orientation)
{
    allwinner::PortalClient* client = allwinner::PortalFactory::GetInstance();
    if(client == NULL)
    {
       _LOGE("call %s get portal client return NULL", __func__);
       return -1;
    }

    client->swapToScreen(0, orientation, 0);
    return 0;
}

int PortalReproj(int mode)
{
    allwinner::PortalClient* client = allwinner::PortalFactory::GetInstance();
    if(client == NULL)
    {
       _LOGE("call %s get portal client return NULL", __func__);
       return -1;
    }

    client->doReprojection(mode);
    return 0;
}
