/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

#ifndef LWM2M_XML_SERDES_H
#define LWM2M_XML_SERDES_H

#include <xmltree.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <inttypes.h>

#include "b64.h"
#include "lwm2m_types.h"
#include "lwm2m_tree_node.h"
#include "lwm2m_debug.h"
#include "lwm2m_list.h"
#include "lwm2m_result.h"
//#include "lwm2m_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Convert a data type string to a AwaResourceType used by LWM2M
AwaResourceType xmlif_StringToDataType(const char * value);

// Convert from a AwaResourceType to a string representation
const char * xmlif_DataTypeToString(AwaResourceType type);

// Convert a string to a Operations enum used by LWM2M
AwaResourceOperations xmlif_StringToOperation(const char * value);

// Convert from an Operations enum to a string representation
const char * xmlif_OperationToString(AwaResourceOperations operation);

int xmlif_GetInteger(TreeNode content, const char * name);

const char * xmlif_GetOpaque(TreeNode content, const char * name);

int xmlif_DecodeValue(char ** dataValue, AwaResourceType dataType, const char * buffer, int bufferLength);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_XML_SERDES_H
