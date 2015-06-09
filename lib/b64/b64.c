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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static const char codes[] =
{
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
        'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
        'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
};

/**
 * @brief write a base64 encoded value to the buffer provided
 *
 * @param[out] out pointer to buffer to write encoded data (must be at least (len + 2) * 4) / 3 bytes
 * @param[in] outLength size of output buffer
 * @param[in] buffer pointer to buffer containing input data
 * @param[in] len length of data in input buffer
 * @return int length of encoded data, -1 on error
 */
int b64Encode(char * out, int outLength, char * buffer, int len)
{
    int i;
    int pos = 0;

    if (out == NULL || buffer == NULL || (outLength < (len + 2) * 4 / 3))
    {
        return -1;
    }

    memset(out, 0, outLength);

    /* convert every 3 bytes to 4 base64 encoded bytes.
     *
     * Text content   | M       | a        |n         |
     * ASCII          |77 (0x4d)| 97 (0x61)|110 (0x6e)|
     * Bit pattern    |010011|01 0110|0001  01|101110 |
     * Index          |   19 |  22   |    5   |   46  |
     * Base64-encoded |   T  |  W    |    F   |    u  |
     */
    for (i = 0; i < len; i += 3)
    {
        uint8_t b;
        // first 6 bits of first byte
        b = (buffer[i] & 0xFC) >> 2;
        out[pos++] = codes[b];
        // last 2 bits of first byte
        b = (buffer[i] & 0x03) << 4;
        if (i + 1 < len)
        {
            // if we have a second byte, then include the next 4 bits
            // of the second byte
            b |= (buffer[i + 1] & 0xF0) >> 4;
            out[pos++] = codes[b];
            // last 4 bits of the second byte
            b = (buffer[i + 1] & 0x0F) << 2;
            if (i + 2 < len)
            {
                // if we have a 3rd byte include the first 2 bits of the 3rd byte
                b |= (buffer[i + 2] & 0xC0) >> 6;
                out[pos++] = codes[b];
                // last 6 bits of the 3rd byte.
                b = buffer[i + 2] & 0x3F;
                out[pos++] = codes[b];
            }
            else
            {
                // pad with =
                out[pos++] = codes[b];
                out[pos++] = '=';
                break;
            }
        }
        else
        {
            // pad with =
            out[pos++] = codes[b];
            out[pos++] = '=';
            out[pos++] = '=';
            break;
        }
    }
    return pos;
}

static int lookupCodeIndex(char c)
{
    int i;
    for (i = 0; i < sizeof(codes); i++)
    {
        if (codes[i] == c)
            return i;
    }
    return -1;
}

/**
 * @brief decode a base64 encoded value and write it to the buffer provided
 *
 * @param[out] out pointer to buffer to write decoded data (must be at least  (len * 3) / 4 bytes
 * @param[in] outLength size of output buffer
 * @param[in] buffer pointer to buffer containing input data
 * @param[in] len length of data in input buffer (must be a multiple of 4)
 * @return int length of decoded data, -1 on error
 */
int b64Decode(char * out, int outLength, char * buffer, int len)
{
    int i;
    int count = 0;
    uint32_t b = 0;
    int pos = 0;

    if (out == NULL || buffer == NULL)
    {
        return -1;
    }

    memset(out, 0, outLength);

    i = 0;
    while (i < len)
    {
        int c = lookupCodeIndex(buffer[i]);
        if (c == -1)
        {
            // RFC 4648 states that we should ignore any characters that are not in the encoding
            // Alphabet, including the padding character.
            i++;
            continue;
        }

        b = (b << 6) | c;
        count ++;

        /* split buffer into 3 bytes */
        if (count == 4)
        {
            if ((pos + 3) > outLength)
            {
                return -1;
            }
            out[pos++] = ((b >> 16) & 0xFF);
            out[pos++] = ((b >> 8) & 0xFF);
            out[pos++] = (b & 0xFF);
            count = 0;
        }
        i++;
    }

    /* handle remaining bytes */
    if (count == 3)
    {
        if (pos + 2 > outLength)
        {
            return -1;
        }
        out[pos++] = ((b >> 10) & 0xFF);
        out[pos++] = ((b >> 2) & 0xFF);
    }
    else if (count == 2)
    {
        if (pos + 1 > outLength)
        {
            return -1;
        }
        out[pos++] = ((b >> 4) & 0xFF);
    }

    return pos;
}
