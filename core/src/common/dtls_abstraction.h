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

/*! \file dtls_abstraction.h
 *  \brief LibFlowCore .
 */


#ifndef DTLS_ABSTRACTION_H_
#define DTLS_ABSTRACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "network_abstraction.h"

typedef enum
{
    NetworkTransmissionError_NotSet = 0,
    NetworkTransmissionError_None,
    NetworkTransmissionError_RecieveBufferEmpty,
    NetworkTransmissionError_TransmitBufferFull,
    NetworkTransmissionError_ConnectionClosed,
    NetworkTransmissionError_ConnectionReset
} NetworkTransmissionError;


typedef NetworkTransmissionError (*DTLS_NetworkSendCallback)(NetworkAddress * destAddress,const uint8_t * buffer, int bufferLength, void *context);

extern const char * DTLS_LibraryName;

void DTLS_Init(void);

void DTLS_Shutdown(void);

bool DTLS_Decrypt(NetworkAddress * sourceAddress, uint8_t * encrypted, int encryptedLength, uint8_t * decryptBuffer, int decryptBufferLength, int * decryptedLength, void *context);

bool DTLS_Encrypt(NetworkAddress * destAddress, uint8_t * plainText, int plainTextLength, uint8_t * encryptedBuffer, int encryptedBufferLength, int * encryptedLength, void *context);

void DTLS_Reset(NetworkAddress * address);

void DTLS_SetCertificate(const uint8_t * cert, int certLength, AwaCertificateFormat format);

void DTLS_SetNetworkSendCallback(DTLS_NetworkSendCallback sendCallback);

void DTLS_SetPSK(const char * identity, const uint8_t * key, int keyLength);

#ifdef __cplusplus
}
#endif

#endif /* DTLS_ABSTRACTION_H_ */
