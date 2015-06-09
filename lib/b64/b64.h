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

#ifndef B64_H
#define B64_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief write a base64 encoded value to the buffer provided
 *
 * @param[out] out pointer to buffer to write encoded data (must be at least (len + 2) * 4) / 3 bytes
 * @param[in] outLength size of output buffer
 * @param[in] buffer pointer to buffer containing input data
 * @param[in] len length of data in input buffer
 * @return int length of encoded data, -1 on error
 */
int b64Encode(char * out, int outLength, char * buffer, int len);

/**
 * @brief decode a base64 encoded value and write it to the buffer provided
 *
 * @param[out] out pointer to buffer to write decoded data (must be at least  (len * 3) / 4 bytes
 * @param[in] outLength size of output buffer
 * @param[in] buffer pointer to buffer containing input data
 * @param[in] len length of data in input buffer (must be a multiple of 4)
 * @return int length of decoded data, -1 on error
 */
int b64Decode(char * out, int outLength, char * buffer, int len);

#ifdef __cplusplus
}
#endif

#endif
