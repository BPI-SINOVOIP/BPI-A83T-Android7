#include "PortalClient.h"

#include <stdio.h>
using namespace allwinner;

int main(void)
{
    PortalClient* client = PortalClient::GetInstance();
    client->prepareFrame(0,left_eye);
    PortalClient::DestroyInstance(client);

    return 0;
}
