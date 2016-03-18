#include <string.h>
#include <stdio.h>
#include "awa/static.h"

int main(void)
{
    AwaStaticClient * awaClient = AwaStaticClient_New();

    AwaStaticClient_SetLogLevel(AwaLogLevel_Error);
    AwaStaticClient_SetEndPointName(awaClient, "AwaStaticClient1");
    AwaStaticClient_SetCOAPListenAddressPort(awaClient, "0.0.0.0", 6000);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coap://[127.0.0.1]:15685");

    AwaStaticClient_Init(awaClient);

    while (1)
    {
        AwaStaticClient_Process(awaClient);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}
