
#include "lwm2m_core.h"

struct _Lwm2mContextType
{
    DefinitionRegistry * Definitions;
};

DefinitionRegistry * Lwm2mCore_GetDefinitions(Lwm2mContextType * context)
{
    return context->Definitions;
}

