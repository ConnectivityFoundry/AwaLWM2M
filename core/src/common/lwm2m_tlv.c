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


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#ifndef CONTIKI
#ifdef MICROCHIP_PIC32
    //#include <tcpip/tcpip_helpers.h>
    #define htonl(x) (((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) | ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24))
    #define ntohl(x) htonl(x)
#else
    #include <arpa/inet.h> // htons
#endif
#else
  #include "net/ip/uip.h"  
  #define htonl(x) uip_htonl(x)
  #define ntohl(x) uip_ntohl(x)
#endif
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#include "lwm2m_tlv.h"
#include "lwm2m_serdes.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"

#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)
#define BIT6 (1 << 6)
#define BIT7 (1 << 7)

#define TLV_MAX_HEADER_SIZE              (6)   // Type + 16 bit Ident + 24 bit Length

/*
 * TLV types
 *
 * Bits 7-6 Indicate the type of identifier
 *   00 - Object Instance in which case the Value contains one or more Resource TLVs
 *   01 - Resource Instance with Value for use within a multiple Resource TLV
 *   10 - Multiple Resource in which case the Value contains one or more Resource Instance TLVs
 *   11 - Resource with value
 */
#define TLV_TYPE_IDENT_MASK                 (BIT7|BIT6)

#define TLV_TYPE_IDENT_OBJECT_INSTANCE         (0)
#define TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE (BIT6)
#define TLV_TYPE_IDENT_MULTIPLE_RESOURCE    (BIT7)
#define TLV_TYPE_IDENT_RESOURCE_VALUE       (BIT7|BIT6)

/*
 * Bit 5 Indicates the Length of the IDentifier
 *    0 - The IDentifier field of this TLV is 8 bits long
 *    1 - The IDentifier field of this TLV is 16 bits long
 */
#define TLV_TYPE_IDENT_LENGTH_MASK       (BIT5)

#define TLV_TYPE_IDENT_LENGTH_8BIT       (0)
#define TLV_TYPE_IDENT_LENGTH_16BIT      (BIT5)

/*
 * Bits 4-3 Indicates the type of Length
 *   00 - No length field the value immediately follows the identifier field and is of the length
 *        Indicated by Bits 2-0
 *   01 - The Length field is 8-bits and Bits 2-0 MUST be ignored
 *   10 - The Length field is 16-bits and Bits 2-0 MUST be ignored
 *   11 - The Length field is 24-bits and Bits 2-0 MUST be ignored 
 */
#define TLV_TYPE_LENGTH_MASK             (BIT4|BIT3)

#define TLV_TYPE_LENGTH_NONE             (0)
#define TLV_TYPE_LENGTH_8BIT             (BIT3)
#define TLV_TYPE_LENGTH_16BIT            (BIT4)
#define TLV_TYPE_LENGTH_24BIT            (BIT4|BIT3)

/*
 * Bits 2-0 - A 3-bit unsigned integer indicating the Length of the Value
 */
#define TLV_TYPE_LENGTH_BITS_MASK       (BIT0|BIT1|BIT2)

/**
 * @brief write a TLV encoded header to the provided buffer
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] length length of data in data section that is to follow the header
 * @return int length of header/index of value
 */
static int TlvEncodeHeader(uint8_t * buffer, int type, uint16_t identifier, int length)
{
    int lengthIndex, valueIndex;

    if (buffer == NULL)
    {
        Lwm2m_Error("Output buffer cannot be NULL\n");
        return -1;
    }

    // set type bits 7-6
    buffer[0] = type & TLV_TYPE_IDENT_MASK;

    // determine identifier length and pack it into the output buffer
    if (identifier <= 0xff)
    {
        buffer[0] |= TLV_TYPE_IDENT_LENGTH_8BIT;
        buffer[1] = identifier;
        lengthIndex = 2;
    }
    else
    {
        buffer[0] |= TLV_TYPE_IDENT_LENGTH_16BIT;
        // 16 bit identifier in big endian
        buffer[1] = (identifier >> 8);
        buffer[2] = identifier;
        lengthIndex = 3;
    }

    // determine type of length
    if (length > 0xffffff)
    {
        // ERROR: length exceeds 24 bits
        Lwm2m_Error("Length exceeds 24 bits\n");
        return -1;
    }
    else if (length > 0xffff)
    {
        buffer[0] |= TLV_TYPE_LENGTH_24BIT;
        // 24 bit length in big endian (network byte order)
        buffer[lengthIndex] = (length >> 16) & 0xff;
        buffer[lengthIndex + 1] = (length >> 8) & 0xff;
        buffer[lengthIndex + 2] = length & 0xff;

        valueIndex = lengthIndex + 3;
    }
    else if (length > 0xff)
    {
        buffer[0] |= TLV_TYPE_LENGTH_16BIT;
        // 16 bit length in big endian
        buffer[lengthIndex] = (length >> 8) & 0xff;
        buffer[lengthIndex + 1] = length & 0xff;

        valueIndex = lengthIndex + 2;
    }
    else if (length > TLV_TYPE_LENGTH_BITS_MASK)
    {
        // 8 bit length
        buffer[0] |= TLV_TYPE_LENGTH_8BIT;
        buffer[lengthIndex] = length;

        valueIndex = lengthIndex + 1;
    }
    else if (length >= 0)
    {
        // in the case where the length will fit in 3 bits, use bits 0-2
        buffer[0] |= TLV_TYPE_LENGTH_NONE;
        buffer[0] |= (length & TLV_TYPE_LENGTH_BITS_MASK);

        valueIndex = lengthIndex;
    }
    else
    {
        // ERROR: negative length
        Lwm2m_Error("Length is negative\n");
        return -1;
    }

    return valueIndex;
}

/**
 * @brief write a TLV encoded header followed by an opaque value to the buffer provided
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] bufferLen size of output buffer
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] value pointer to buffer containing opaque data
 * @param[in] length length of data in value buffer
 * @return int length of header + opaque data, or -1 on error
 */
static int TlvEncodeOpaque(uint8_t * buffer, int bufferLen, int type, int id, uint8_t * value, int len)
{
    uint8_t header[TLV_MAX_HEADER_SIZE];
    int headerLen;

    if ((buffer == NULL) || (len > 0 && value == NULL))
    {
        Lwm2m_Error("Input or output buffers cannot be NULL\n");
        return -1;
    }

    headerLen = TlvEncodeHeader(&header[0], type, id, len);
    if (headerLen == -1)
    {
        Lwm2m_Error("Failed to encode TLV header\n");
        return -1;
    }

    if (bufferLen < (headerLen + len))
    {
        Lwm2m_Error("Output buffer is too small to encode data\n");
        return -1;
    }

    memmove(buffer + headerLen, value, len);
    memcpy(buffer, header, headerLen);
    return headerLen + len;
}

/**
 * @brief write a TLV encoded header followed by a TLV encoded integer
 *
 * TLV encoded integers are represented as a binary signed integer in network byte order,
 * where the first (most significant) bit is 0 for a positive integer and 1 for a negative integer. 
 * The value may be 1 (8-bit), 2 (16-bit), 4 (32-bit) or 8 (64-bit) bytes long
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] bufferLen size of output buffer
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] value integer 8-64 bit 
 * @return int length of header + integer data
 */
static int TlvEncodeInteger(uint8_t * buffer, int bufferLen, int type, int id, int64_t value)
{
    uint8_t valueBuffer[sizeof(int64_t)] = {0};  // allow up to 64bit ints
    int intSize, i;
 
#ifdef LWM2M_V1_0   
    bool negative = (value < 0);

    // remove any sign extension from value
    uint64_t uValue = (negative) ? -value : value;
#endif

    // determine minimum number of bytes required to encode the provided value
    if (value >= -128 && value <= 127)
    {
        intSize = 1;
    }
    else if (value >= -32768 && value <= 32767)
    {
        intSize = 2;
    }
    else if (value >= -2147483647 && value <= 2147483647)
    {
        intSize = 4;
    }
    else
    {
        intSize = 8;
    }

#ifdef LWM2M_V1_0
    // serialise the integer value, in network byte order
    for (i = 0; i < intSize; i++)
    {
        valueBuffer[(intSize - 1) - i] = (uValue >> (8 * i)) & 0xff;
    }

    if (negative)
    {
        // as per the LWM2M spec, set the most significant bit to 1 for negative
        // integers
        valueBuffer[0] |= 0x80;
    }
#else
    // serialise the integer value, in network byte order
    for (i = 0; i < intSize; i++)
    {
        valueBuffer[(intSize - 1) - i] = (value >> (8 * i)) & 0xff;
    }
#endif

    // once encoded, we can just treat this as opaque
    return TlvEncodeOpaque(buffer, bufferLen, type, id, valueBuffer, intSize);
}

/**
 * @brief write a TLV encoded header followed by an encoded float
 *
 * TLV encoded floats are represented as an [IEEE 754-2008] [FLOAT] 
 * binary floating point value. The value may use the binary32 (4 byte Length)
 * or binary64 (8 byte Length) format
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] bufferLen size of output buffer
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] value float/double 32 or 64 bit 
 * @return int length of header + integer data
 */
static int TlvEncodeFloat(uint8_t * buffer, int bufferLen, int type, int id, double value)
{
    uint8_t valueBuffer[sizeof(uint64_t)];

    // decision to use float or double depends on magnitude and precision.
    int floatSize = (float)value != value ? 8 : 4;

    if (floatSize == 4)
    {
        float f = value;
        int32_t temp = htonl(*(uint32_t*)&f);
        memcpy(valueBuffer, &temp, floatSize);
    }
    else if (floatSize == 8)
    {
        int64_t temp = htonll(*(uint64_t*)&value);
        memcpy(valueBuffer, &temp, floatSize);
    }
    // once encoded, we can just treat this as opaque
    return TlvEncodeOpaque(buffer, bufferLen, type, id, valueBuffer, floatSize);
}

/**
 * @brief write a TLV encoded header followed by a boolean to the buffer provided
 *
 * TLV encoded booleans are represented as an Integer with value 0, or 1. 
 * with a length of 1
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] bufferLen size of output buffer
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] value boolean value to write to buffer
 * @return int length of header + 1 byte for the boolean value
 */
static int TlvEncodeBoolean(uint8_t * buffer, int bufferLen, int type, int id, bool value)
{
    return TlvEncodeInteger(buffer, bufferLen, type, id, value);
}

/**
 * @brief write a TLV encoded header followed by an encoded objectlink value
 *
 * ObjectLink values are represented as 2 16-bits Integer one beside the other.
 *  The first one represents the ObjectID, and the second one represents the 
 * ObjectInstanceID. This value is always 4 bytes long.
 *
 *
 * @param[out] buffer pointer to buffer to write encoded tlv header
 * @param[in] bufferLen size of output buffer
 * @param[in] type TLV identifier type, one of: TLV_TYPE_IDENT_OBJECT_INSTANCE,
 *                                              TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE,
 *                                              TLV_TYPE_IDENT_MULTIPLE_RESOURCE,
 *                                              TLV_TYPE_IDENT_RESOURCE_VALUE
 * @param[in] identifier identifier value 0-65535
 * @param[in] objectID objectID value to write to buffer
 * @param[in] objectInstanceID objectInstanceID value to write to buffer
 * @return int length of header + 4 byte object link value
 */
static int TlvEncodeObjectLink(uint8_t * buffer, int bufferLen, int type, int id, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID)
{
    uint8_t valueBuffer[4];

    valueBuffer[0] = (objectID >> 8) & 0xff;
    valueBuffer[1] = objectID & 0xff;
    valueBuffer[2] = (objectInstanceID >> 8) & 0xff;
    valueBuffer[3] = objectInstanceID & 0xff;

    return TlvEncodeOpaque(buffer, bufferLen, type, id, valueBuffer, sizeof(valueBuffer));
}

/**
 * @brief decode a TLV encoded header
 *
 * @param[out] type pointer to int to store TLV identifier type
 * @param[out] identifier pointer to int to store decoded identifier value
 * @param[out] length pointer to int to store decoded length
 * @param[in] buffer pointer to buffer containing TLV encoded header
 * @param[in] bufferLen length of buffer to decode
 * @return int position of value or -1 on error
 */
static int TlvDecodeHeader(int * type, uint16_t * identifier, int * length, const uint8_t * buffer, int bufferLen)
{
    int valueIndex;
    int lengthIndex;

    if ((type == NULL) || (identifier == NULL) || (length == NULL) || (buffer == NULL))
    {
        Lwm2m_Error("Input or output buffers cannot be NULL\n");
        return -1;
    }
    
    if (bufferLen < 2)
    {
        Lwm2m_Error("Buffer length is too small\n");
        return -1;
    }

    *type = buffer[0] & TLV_TYPE_IDENT_MASK;

    // determine identifier length
    if ((buffer[0] & TLV_TYPE_IDENT_LENGTH_MASK) == TLV_TYPE_IDENT_LENGTH_16BIT)
    {
        if (bufferLen < 3)
        {
            Lwm2m_Error("Buffer length is too small\n");
            return -1;
        }

        // 16 bit identifier
        *identifier = (buffer[1] << 8) | buffer[2];
        lengthIndex = 3;
    }
    else
    {
        // 8 bit identifier
        *identifier = buffer[1];
        lengthIndex = 2;
    }

    // determine number of length bytes used 0-3
    switch(buffer[0] & TLV_TYPE_LENGTH_MASK)
    {
    case TLV_TYPE_LENGTH_24BIT:
        if (bufferLen < (lengthIndex + 2))
        {
            Lwm2m_Error("Buffer length is too small\n");
            return -1;
        }
        *length = (buffer[lengthIndex] << 16) |
                  (buffer[lengthIndex + 1] << 8) |
                  buffer[lengthIndex + 2];
        valueIndex = lengthIndex + 3;
        break;

    case TLV_TYPE_LENGTH_16BIT:
        if (bufferLen < lengthIndex + 1)
        {
            Lwm2m_Error("Buffer length is too small\n");
            return -1;
        }
        *length = (buffer[lengthIndex] << 8) |
                  buffer[lengthIndex + 1];
        valueIndex = lengthIndex + 2;
        break;

    case TLV_TYPE_LENGTH_8BIT:
        if (bufferLen < lengthIndex)
        {
            Lwm2m_Error("Buffer length is too small\n");
            return -1;
        }
        *length = buffer[lengthIndex];
        valueIndex = lengthIndex + 1;
        break;

    case TLV_TYPE_LENGTH_NONE:
        *length = buffer[0] & TLV_TYPE_LENGTH_BITS_MASK;
        valueIndex = lengthIndex;
        break;

    default:
        Lwm2m_Error("Invalid data length\n");
        return -1;
    }

    return valueIndex;
}

/**
 * @brief decode a TLV encoded float value (excluding the header)
 *
 * @param[out] dest pointer to double to store decoded float
 * @param[in] buffer pointer to buffer containing TLV encoded header
 * @param[in] size length of buffer to decode, 4 or 8 bytes
 * @return 0 on success or -1 on error
 */
static int TlvDecodeFloat(double * dest, const uint8_t * buffer, int size)
{
    if ((dest == NULL) || (buffer == NULL))
    {
        Lwm2m_Error("Input or ouput buffers cannot be NULL\n");
        return -1;
    }

    if (size == 4)
    {
        float f;
        int32_t temp = ntohl(*(uint32_t*)buffer);
        memcpy(&f, &temp, sizeof(temp));
        *dest = f;
    }
    else if (size == 8)
    {
        int64_t temp = ntohll(*(uint64_t*)buffer);
        memcpy(dest, &temp, sizeof(temp));
    }
    else
    {
        Lwm2m_Error("Invalid size, should be either 4 or 8 bytes: received %d\n", size);
        return -1;
    }

    return 0;
}

/**
 * @brief decode a TLV encoded integer value (excluding the header)
 *
 * @param[out] dest pointer to int64_t to store decoded int
 * @param[in] buffer pointer to buffer containing TLV encoded header
 * @param[in] size length of buffer to decode, 1,2, 4 or 8 bytes
 * @return 0 on success or -1 on error
 */
static int TlvDecodeInteger(int64_t * dest, const uint8_t * buffer, int size)
{
    int i;
#ifdef LWM2M_V1_0
    bool negative;
#endif

    if ((dest == NULL) || (buffer == NULL))
    {
        Lwm2m_Error("Input or output buffers cannot be NULL\n");
        return -1;
    }

    // check for valid range
    if ((size != 1) && (size != 2) && (size != 4) && (size != 8))
    {
        Lwm2m_Error("Invalid size, should be 1,2,4 or 8 bytes\n");
        return -1;
    }

#ifdef LWM2M_V1_0
    // extract the signedness from the first bit
    negative = buffer[0] & 0x80;

    // read the first byte, mask out the signedness
    *dest = buffer[0] & 0x7f;
#else
    *dest = (*(int8_t*)&buffer[0]);
#endif

    for (i = 1; i < size; i++)
    {
        *dest = (*dest << 8) | buffer[i];
    } 

#ifdef LWM2M_V1_0
    if (negative)
    {
        *dest = -*dest;
    }
#endif

    return 0;
}

/**
 * @brief decode a TLV encoded object link value (excluding the header)
 *
 * @param[out] objectID pointer to uint16_t to store decoded objectID
 * @param[out] instanceID pointer to uint16_t to store decoded instanceID
 * @param[in] buffer pointer to buffer containing TLV encoded header
 * @param[in] size length of buffer to decode, must be 4 bytes
 * @return 0 on success or -1 on error
 */
static int TlvDecodeObjectLink(AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, const uint8_t * buffer, int size)
{
    if ((objectID == NULL) || (objectInstanceID == NULL) || (buffer == NULL))
    {
        Lwm2m_Error("Input or output buffers cannot be NULL\n");
        return -1;
    }

    if (size != 4)
    {
        Lwm2m_Error("Invalid size, should be 4 bytes\n");
        return -1;
    }

    *objectID = (buffer[0] << 8) | buffer[1];
    *objectInstanceID = (buffer[2] << 8) | buffer[3];

    return 0;
}

/**
 * @brief write a TLV encoded resource instance to the buffer provided
 *
 * @param[in] node tree node containing resource instance value from the object store
 * @param[in] definition format of the resource containing this resource instance
 * @param[in] objectID
 * @param[in] objectInstanceID
 * @param[in] resourceID
 * @param[in] resourceInstanceID
 * @param[out] buffer pointer to buffer to store resulting data
 * @param[in] len length of buffer
 * @return int length of serialised data, or -1 on error
 */
static int TlvSerialiseResourceInstance(Lwm2mTreeNode * node, ResourceDefinition * definition, ObjectIDType objectID,
                                        ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, uint8_t * buffer, int len)
{
    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ResourceInstance)
    {
       Lwm2m_Error("Resource Instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    int valueLength = -1;
    int id;
    int type;
    uint16_t size;
    uint8_t * value;

    value = (uint8_t *)Lwm2mTreeNode_GetValue(node, &size);
    if (value == NULL)
    {
        switch (definition->Type)
        {
            case AwaResourceType_String:  // no break
            case AwaResourceType_Opaque:
                size = 0; // This is ok: just means we have an empty string.
                break;
            default:
                Lwm2m_Error("Resource instance value is NULL: /%d/%d/%d/%d\n", objectID, objectInstanceID, resourceID, resourceInstanceID);
                return -1;
        }

    }

    if (IS_MULTIPLE_INSTANCE(definition))
    {
        type = TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE;
        id = resourceInstanceID;
    }
    else
    {
        type = TLV_TYPE_IDENT_RESOURCE_VALUE;
        id = resourceID;
    }

    switch (definition->Type)
    {

        case AwaResourceType_Boolean:
            valueLength = TlvEncodeBoolean(buffer, len, type, id, *(bool*)value);
            break;

        case AwaResourceType_Time: // no break
        case AwaResourceType_Integer:
            switch (size)
            {
                case sizeof(int8_t):
                    valueLength = TlvEncodeInteger(buffer, len, type, id, ptrToInt8(value));
                    break;
                case sizeof(int16_t):
                    valueLength = TlvEncodeInteger(buffer, len, type, id, ptrToInt16(value));
                    break;
                case sizeof(int32_t):
                    valueLength = TlvEncodeInteger(buffer, len, type, id, ptrToInt32(value));
                    break;
                case sizeof(int64_t):
                    valueLength = TlvEncodeInteger(buffer, len, type, id, ptrToInt64(value));
                    break;
                default:
                    break;
            }
            break;

        case AwaResourceType_Float:
            switch (size)
            {
                case sizeof(float):
                    valueLength = TlvEncodeFloat(buffer, len, type, id, *(float*)value);
                    break;
                case sizeof(double):
                    valueLength = TlvEncodeFloat(buffer, len, type, id, *(double*)value);
                    break;
                default:
                    Lwm2m_Error("Invalid length for float: %d\n", size);
                    break;
            }
            break;

        case AwaResourceType_String:
        case AwaResourceType_Opaque:
            valueLength = TlvEncodeOpaque(buffer, len, type, id, (uint8_t*)value, size);
            break;

        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink * objectLink = (AwaObjectLink *) value;
                Lwm2m_Debug("Object ID %d Object Instance %d\n", objectLink->ObjectID, objectLink->ObjectInstanceID);
                valueLength = TlvEncodeObjectLink(buffer, len, type, id, objectLink->ObjectID, objectLink->ObjectInstanceID);
            }
            break;
        default:
            Lwm2m_Error("Unknown type: %d\n", definition->Type);
            break;
    }

    return valueLength;
}

/**
 * @brief write a TLV encoded resource to the buffer provided
 *
 * @param[in] tree node containing resource from the object store
 * @param[in] objectID
 * @param[in] objectInstanceID
 * @param[in] resourceID
 * @param[out] buffer pointer to buffer to store resulting data
 * @param[in] len length of buffer
 * @return int length of serialised data, or -1 on error
 */
static int TlvSerialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode * node, const ObjectIDType objectID,
                                ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len)
{
    int resourceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Resource)
    {
       Lwm2m_Error("Resource node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    ResourceDefinition * definition = (ResourceDefinition*)Lwm2mTreeNode_GetDefinition(node);

    if (definition == NULL)
    {
        Lwm2m_Error("No resource definition for Object %d Instance %d Resource %d\n", objectID, objectInstanceID, resourceID);
        return -1;
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
       int resourceInstanceID;
       Lwm2mTreeNode_GetID(child, &resourceInstanceID);

       int valueLength = TlvSerialiseResourceInstance(child, definition, objectID, objectInstanceID, resourceID, resourceInstanceID, &buffer[resourceLength], len - resourceLength);
       if (valueLength <= 0)
       {
           Lwm2m_Error("Failed to serialise resource instance /%d/%d/%d/%d valueLength = %d\n", objectID, objectInstanceID, resourceID, resourceInstanceID, valueLength);
           return -1;
       }
       resourceLength += valueLength;
       child = Lwm2mTreeNode_GetNextChild(node, child);
    }

    if (IS_MULTIPLE_INSTANCE(definition))
    {
       uint8_t header[TLV_MAX_HEADER_SIZE];
       int headerLen;
       // Add Mutliple resource instance header
       headerLen = TlvEncodeHeader(&header[0], TLV_TYPE_IDENT_MULTIPLE_RESOURCE, resourceID, resourceLength);
       if (headerLen == -1)
       {
           Lwm2m_Error("Failed to encode TLV header\n");
           return -1;
       }
       memmove(buffer + headerLen, buffer, resourceLength);
       memcpy(buffer, header, headerLen);
       resourceLength += headerLen;
    }
    return resourceLength;
}

/**
 * @brief write a TLV encoded instance to the buffer provided, 
 * this function should be used to encode single instances only
 *
 * @param[in] node tree node containing object instance from the object store
 * @param[in] objectID
 * @param[in] objectInstanceID
 * @param[out] buffer pointer to buffer to store resulting data
 * @param[in] len length of buffer
 * @return int length of serialised data, or -1 on error
 */
static int TlvSerialiseObjectInstance(SerdesContext * serdesContext, Lwm2mTreeNode * node, const ObjectIDType objectID,
                                      ObjectInstanceIDType objectInstanceID, uint8_t * buffer, int len)
{
    int instanceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ObjectInstance)
    {
        Lwm2m_Error("Object instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        int resourceID;
        Lwm2mTreeNode_GetID(child, &resourceID);

        int resourceLength = TlvSerialiseResource(serdesContext, child, objectID, objectInstanceID, resourceID, &buffer[instanceLength], len - instanceLength);
        if (resourceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise resource\n");
            return -1;
        }
        instanceLength += resourceLength;

        child = Lwm2mTreeNode_GetNextChild(node, child);
    }
    return instanceLength;
}

/**
 * @brief write a TLV encoded object to the buffer provided
 *
 * @param[in] tree node containing object from the object store
 * @param[in] object object tree to serialise
 * @param[out] buffer pointer to buffer to store resulting data
 * @param[in] len length of buffer
 * @return int length of serialised data, or -1 on error
 */
static int TlvSerialiseObject(SerdesContext * serdesContext, Lwm2mTreeNode * node, const ObjectIDType objectID, uint8_t * buffer, int len)
{
    int pos = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Object)
    {
        Lwm2m_Error("Object node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        int objectInstanceID;
        uint8_t header[TLV_MAX_HEADER_SIZE];
        int headerLen;

        Lwm2mTreeNode_GetID(child, &objectInstanceID);

        int instanceLength = TlvSerialiseObjectInstance(serdesContext, child, objectID, objectInstanceID, &buffer[pos], len - pos);
        if (instanceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise object instance\n");
            return -1;
        }

        // if there are multiple object instances, then we need to add an object instance header.
        // For single object instances we can skip this step, and exit from the object instance loop.
        headerLen = TlvEncodeHeader(&header[0], TLV_TYPE_IDENT_OBJECT_INSTANCE, objectInstanceID, instanceLength);
        if (headerLen == -1)
        {
            Lwm2m_Error("Failed to encode TLV header\n");
            return -1;
        }

        memmove(&buffer[pos] + headerLen, &buffer[pos], instanceLength);
        memcpy(&buffer[pos], header, headerLen);

        pos += (instanceLength + headerLen);

        child = Lwm2mTreeNode_GetNextChild(node, child);
    }

    return pos;
}

/**
 * @brief deserialise the TLV encoded data provided 
 *
 * @param[in] buffer pointer to TLV serialised buffer
 * @param[in] length length of buffer
 * @return int -1 on error
 */
static int TlvDeserialiseResourceInstance(Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                                          ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, int resID, const uint8_t * buffer, int len)
{
    int result = -1;

    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, resID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_ResourceInstance);

    AwaResourceType resourceType = Definition_GetResourceType(registry, objectID, resourceID);
    switch (resourceType)
    {
        case AwaResourceType_Integer:
        case AwaResourceType_Time:
        case AwaResourceType_Boolean:
            {
                int64_t temp = 0;
                result = TlvDecodeInteger((int64_t*)&temp, buffer, len);
                if (result >= 0)
                {
                    if (resourceType != AwaResourceType_Boolean)
                    {
                        Lwm2mTreeNode_SetValue(*dest, (const uint8_t*)&temp, sizeof(int64_t));
                    }
                    else
                    {
                        bool tempBool = temp == 0 ? false : true;
                        Lwm2mTreeNode_SetValue(*dest, (const uint8_t*)&tempBool, sizeof(bool));
                    }
                }
            }
            break;
        case AwaResourceType_Float:
            {
                double temp = 0;
                result = TlvDecodeFloat((double*)&temp, buffer, len);
                if (result >= 0)
                {
                    Lwm2mTreeNode_SetValue(*dest, (const uint8_t*)&temp, sizeof(double));
                }
            }
            break;
        case AwaResourceType_String:
            {
                Lwm2mTreeNode_SetValue(*dest, buffer, len);
                result = 0;
            }
            break;
        case AwaResourceType_Opaque:
            Lwm2mTreeNode_SetValue(*dest, buffer, len);
            result = 0;
            break;
        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink temp;
                result = TlvDecodeObjectLink(&temp.ObjectID, &temp.ObjectInstanceID, buffer, len);

                if(result >= 0)
                {
                    Lwm2mTreeNode_SetValue(*dest, (const uint8_t*)&temp, sizeof(AwaObjectLink));
                }
                break;
            }
        default:
            Lwm2m_Error("Unknown type: %d\n", resourceType);
            break;
    }

    return result;
}

/**
 * @brief deserialise the TLV encoded data provided 
 *
 * @param[in] buffer pointer to TLV serialised buffer
 * @param[in] length length of buffer
 * @return int -1 on error
 */
static int TlvDeserialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                                  ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const uint8_t * buffer, int bufferLen)
{
    int type, resourceLen, headerLen;
    uint16_t identifier;
    ResourceDefinition * definition;

    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, resourceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Resource);

    definition = Definition_LookupResourceDefinition(registry, objectID, resourceID);

    if (definition == NULL)
    {
       Lwm2m_Error("Failed to determine resource definition Object %d Resource %d\n", objectID, resourceID);
       return -1;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("Failed to set definition Object %d Resource %d\n", objectID, resourceID);
        return -1;
    }

    // peek in buffer to check to see if we are dealing with a resource, or multiple resource 
    headerLen = TlvDecodeHeader(&type, &identifier, &resourceLen, buffer, bufferLen);
    if (headerLen == -1)
    {
        Lwm2m_Error("Failed to decode TLV header\n");
        return -1;
    }

    // the header must either be a resource value, or a multiple resource
    if (type == TLV_TYPE_IDENT_RESOURCE_VALUE)
    {
        Lwm2mTreeNode * resourceValueNode;
        int result = TlvDeserialiseResourceInstance(&resourceValueNode, registry, objectID, objectInstanceID, resourceID, 0, &buffer[headerLen], resourceLen);
        if (result != -1)
        {
            Lwm2mTreeNode_AddChild(*dest, resourceValueNode);
        }
        else
        {
            Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
        }
        return result;
    }
    else if (type == TLV_TYPE_IDENT_MULTIPLE_RESOURCE)
    {
        // skip on to next header
        int pos = 0;
        const uint8_t * resourceBuffer = &buffer[headerLen];

        if ((bufferLen - headerLen) < resourceLen)
        {
            Lwm2m_Error("Cannot deserialise resource, buffer too short\n");
            return -1;
        }

        while (pos < resourceLen)
        {
            int result, length, valueIndex;
            Lwm2mTreeNode * resourceValueNode;

            // peek in buffer to check to see if we are dealing with a resource, or and object instance.
            valueIndex = TlvDecodeHeader(&type, &identifier, &length, &resourceBuffer[pos], resourceLen - pos);

            if (valueIndex == -1)
            {
                return -1;
            }

            if (length > (resourceLen - pos))
            {
                Lwm2m_Error("Cannot deserialise resource, buffer too short\n");
                return -1;
            }

            // the header must either be a resource value, or a multiple resource
            if (type != TLV_TYPE_IDENT_MULTI_RESOURCE_VALUE)
            {
                Lwm2m_Error("Cannot deserialise resource, malformed tlv\n");
                return -1;
            }

            pos += valueIndex;

            result = TlvDeserialiseResourceInstance(&resourceValueNode, registry, objectID, objectInstanceID, resourceID, identifier, &resourceBuffer[pos], length);

            if (result == -1)
            {
                Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                Lwm2m_Error("Failed to deserialise resource instance\n");
                return -1;
            }

            Lwm2mTreeNode_AddChild(*dest, resourceValueNode);

            pos += length;
        }

        return pos;
    }

    Lwm2m_Error("Malformed TLV, unexpected type 0x%X, ident 0x%X resource length %d header len %d\n", type, identifier, resourceLen, headerLen);
    return -1;
}

/**
 * @brief deserialise the TLV encoded data provided
 *
 * @param[in] buffer pointer to TLV serialised buffer
 * @param[in] length length of buffer
 * @return int -1 on error
 */
static int TlvDeserialiseObjectInstance(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                                        ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, const uint8_t * buffer, int bufferLen)
{
    int pos = 0;

    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, objectInstanceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_ObjectInstance);

    ObjectDefinition * definition = Definition_LookupObjectDefinition(registry, objectID);
    if (definition == NULL)
    {
       Lwm2m_Error("Failed to determine object definition Object %d\n", objectID);
       return -1;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("Failed to set definition Object %d\n", objectID);
        return -1;
    }

    while (pos < bufferLen)
    {
        int type, length, headerLen;
        uint16_t identifier;

        // peek in buffer to check to see if we are dealing with a resource, or an object instance.
        headerLen = TlvDecodeHeader(&type, &identifier, &length, &buffer[pos], bufferLen - pos);
        if (headerLen == -1)
        {
            Lwm2m_Error("Failed to decode TLV header\n");
            return -1;
        }

        if (type == TLV_TYPE_IDENT_OBJECT_INSTANCE)
        {
            // support resources encapsulated in a single object instance - although the sender shouldn't really do this (instance ID should be in the path not the payload...)
            if (identifier != objectInstanceID)
            {
                Lwm2m_Error("Object instance ID specified in TLV object instance header(%d) does not match ID specified in path(%d)\n", identifier, objectInstanceID);
                return -1;
            }
            pos += headerLen;
            continue;
        }

        // the header must either be a resource value, or a multiple resource
        if (type == TLV_TYPE_IDENT_RESOURCE_VALUE || type == TLV_TYPE_IDENT_MULTIPLE_RESOURCE)
        {
            int result;
            Lwm2mTreeNode * resourceNode;
            result = TlvDeserialiseResource(serdesContext, &resourceNode, registry, objectID, objectInstanceID, identifier, &buffer[pos], bufferLen - pos);
            if (result < 0)
            {
                Lwm2mTreeNode_DeleteRecursive(resourceNode);
                return -1;
            }
            Lwm2mTreeNode_AddChild(*dest, resourceNode);
        }
        else
        {
            // malformed
            return -1;
        }

        pos += (headerLen + length);
    }

    return pos;
}

/**
 * @brief deserialise the TLV encoded data provided 
 *
 * @param[in] buffer pointer to TLV serialised buffer
 * @param[in] length length of buffer
 * @return int -1 on error
 */
static int TlvDeserialiseObject(SerdesContext * serdesContext, Lwm2mTreeNode ** dest,
        const DefinitionRegistry * registry, ObjectIDType objectID, const uint8_t * buffer, int bufferLen)
{
    int pos = 0;
    ObjectDefinition * definition;
    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, objectID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Object);

    definition = Definition_LookupObjectDefinition(registry, objectID);

    if (definition == NULL)
    {
       Lwm2m_Error("Failed to determine resource definition Object %d\n", objectID);
       return -1;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("Failed to set definition Object %d\n", objectID);
        return -1;
    }

    while (pos < bufferLen)
    {

        int type, length, headerLen, result;
        uint16_t identifier;

        // peek in buffer to check to see if we are dealing with a resource, or and object instance.
        headerLen = TlvDecodeHeader(&type, &identifier, &length, &buffer[pos], bufferLen - pos);
        if (headerLen == -1)
        {
            Lwm2m_Error("Failed to decode TLV header\n");
            return -1;
        }

        if (type == TLV_TYPE_IDENT_OBJECT_INSTANCE)
        {
            // multiple instance object
            Lwm2mTreeNode * instanceNode;

            // strip off the object instance header, pass instanceID into function.
            pos += headerLen;
            result = TlvDeserialiseObjectInstance(serdesContext, &instanceNode, registry, objectID, identifier, &buffer[pos], length);

            if(result > 0)
            {
                Lwm2mTreeNode_AddChild(*dest, instanceNode);
                pos += result;
            }
            else
            {
                Lwm2mTreeNode_DeleteRecursive(instanceNode);
                return result;
            }
        }
        else
        {
            if (pos == 0)
            {
                // single instance object - check to see if this is just an instance and not an object
                // case where we receive a "CREATE" with no object instance ID (client should generate it)
                Lwm2mTreeNode * instanceNode;
                ObjectInstanceIDType objectInstanceID = -1;  // instance ID will be generated
                result = TlvDeserialiseObjectInstance(serdesContext, &instanceNode, registry, objectID, objectInstanceID, buffer, bufferLen);
                if (result > 0)
                {
                    Lwm2mTreeNode_AddChild(*dest, instanceNode);
                }
                else
                {
                    Lwm2mTreeNode_DeleteRecursive(instanceNode);
                }
            }
            else
            {
                // malformed TLV
                result = -1;
            }
            return result;
        }
    }

    return 0;
}

// Map TLV serdes function delegates
const SerialiserDeserialiser tlvSerDes =
{
    .SerialiseObject           = TlvSerialiseObject,
    .SerialiseObjectInstance   = TlvSerialiseObjectInstance,
    .SerialiseResource         = TlvSerialiseResource,
    .DeserialiseObject         = TlvDeserialiseObject,
    .DeserialiseObjectInstance = TlvDeserialiseObjectInstance,
    .DeserialiseResource       = TlvDeserialiseResource,
};

