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
#include <stdint.h>
#include <string.h>

/************************************************************************************************************
 * Definitions
 ************************************************************************************************************/
#define SHA256_HASH_LENGTH 32
#define MAX_KEY_LENGTH     64

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b)  (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)     (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)     (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)    (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)    (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8_t) ( (n) >> 24 );     \
    (b)[(i) + 1] = (uint8_t) ( (n) >> 16 );     \
    (b)[(i) + 2] = (uint8_t) ( (n) >>  8 );     \
    (b)[(i) + 3] = (uint8_t) ( (n)       );     \
}

/************************************************************************************************************
 * Typedefs
 ************************************************************************************************************/
typedef struct
{
    uint8_t  Data[64];
    uint32_t DataLen;
    uint32_t BitLen[2];
    uint32_t State[8];

} Sha256ContextType;

/************************************************************************************************************
 * Implementation
 ************************************************************************************************************/
static const uint32_t k[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void Sha256_Transform(Sha256ContextType *context, uint8_t * data)
{
    uint32_t a,b,c,d,e,f,g,h,i,j,m[64];

    for (i = 0,j = 0; i < 16; ++i, j += 4)
    {
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    }

    for (; i < 64; i++)
    {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }

    a = context->State[0];
    b = context->State[1];
    c = context->State[2];
    d = context->State[3];
    e = context->State[4];
    f = context->State[5];
    g = context->State[6];
    h = context->State[7];

    for (i = 0; i < 64; i++)
    {
        uint32_t t1, t2;
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    context->State[0] += a;
    context->State[1] += b;
    context->State[2] += c;
    context->State[3] += d;
    context->State[4] += e;
    context->State[5] += f;
    context->State[6] += g;
    context->State[7] += h;
}

static void Sha256_Init(Sha256ContextType *context)
{
    context->DataLen   = 0;
    context->BitLen[0] = 0;
    context->BitLen[1] = 0;
    context->State[0]  = 0x6a09e667;
    context->State[1]  = 0xbb67ae85;
    context->State[2]  = 0x3c6ef372;
    context->State[3]  = 0xa54ff53a;
    context->State[4]  = 0x510e527f;
    context->State[5]  = 0x9b05688c;
    context->State[6]  = 0x1f83d9ab;
    context->State[7]  = 0x5be0cd19;
}

static void Sha256_Update(Sha256ContextType *context, const uint8_t * data, uint32_t dataLen)
{
    uint32_t i;

    for (i = 0; i < dataLen; i++)
    {
        context->Data[context->DataLen] = data[i];
        context->DataLen++;
        if (context->DataLen == 64)
        {
            Sha256_Transform(context,context->Data);
            DBL_INT_ADD(context->BitLen[0], context->BitLen[1], 512);
            context->DataLen = 0;
        }
    }
}

static void Sha256_Final(Sha256ContextType *context, uint8_t hash[])
{
    uint32_t i;

    i = context->DataLen;

    // Pad whatever data is left in the buffer.
    if (context->DataLen < 56)
    {
        context->Data[i++] = 0x80;
        while (i < 56)
            context->Data[i++] = 0x00;
    }
    else
    {
        context->Data[i++] = 0x80;
        while (i < 64)
            context->Data[i++] = 0x00;

        Sha256_Transform(context,context->Data);
        memset(context->Data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    DBL_INT_ADD(context->BitLen[0], context->BitLen[1], context->DataLen * 8);
    PUT_UINT32(context->BitLen[0], context->Data, 60);
    PUT_UINT32(context->BitLen[1], context->Data, 56);
    Sha256_Transform(context, context->Data);

    // Copy result into output buffer
    PUT_UINT32(context->State[0], hash,  0);
    PUT_UINT32(context->State[1], hash,  4);
    PUT_UINT32(context->State[2], hash,  8);
    PUT_UINT32(context->State[3], hash, 12);
    PUT_UINT32(context->State[4], hash, 16);
    PUT_UINT32(context->State[5], hash, 20);
    PUT_UINT32(context->State[6], hash, 24);
    PUT_UINT32(context->State[7], hash, 28);
}

/**
 * @brief Compute a Hash of the data provided using SHA256 and write the result into a buffer
 * @param[out] hash   - buffer to store resulting hash
 * @param[in] data    - pointer to data to hash
 * @param[in] dataLen - length of data in buffer
 */
static void Sha256_ComputeHash(uint8_t hash[SHA256_HASH_LENGTH], const uint8_t * data, int dataLen)
{
    Sha256ContextType context;
    Sha256_Init(&context);
    Sha256_Update(&context,data, dataLen);
    Sha256_Final(&context, hash);
}

/**
 * @brief Compute a Hmac using SHA256 of the data provided and write the result into a buffer
 * @param[out] hash   - buffer to store resulting hash
 * @param[in] data    - pointer to data to hash
 * @param[in] dataLen - length of data in buffer
 * @param[in] key     - pointer to key
 * @param[in] keyLen  - length of key
 */
void HmacSha256_ComputeHash(uint8_t hash[SHA256_HASH_LENGTH], const uint8_t * data, int dataLen, const uint8_t * key, int keyLen)
{
    uint8_t innerKeyPad[MAX_KEY_LENGTH];
    uint8_t outerKeyPad[MAX_KEY_LENGTH];
    uint8_t keyHash[SHA256_HASH_LENGTH];
    uint8_t innerHash[SHA256_HASH_LENGTH];
    uint8_t buffer[1024]; // FIXME: maximum data length is currently 1024 - 64 bytes
    int i;

    // if key is longer than 64 bytes use the hash of the key as the key.
    if (keyLen > MAX_KEY_LENGTH)
    {
        Sha256_ComputeHash(keyHash, key, keyLen);
        key    = keyHash;
        keyLen = SHA256_HASH_LENGTH;
    }

    // copy key to pad values, we will then XOR this with 0x36 and 0x5c respectively
    memset(innerKeyPad, 0, sizeof(innerKeyPad));
    memset(outerKeyPad, 0, sizeof(outerKeyPad));
    memcpy(innerKeyPad, key, keyLen);
    memcpy(outerKeyPad, key, keyLen);

    for (i = 0; i < MAX_KEY_LENGTH; i++)
    {
        innerKeyPad[i] ^= 0x36;
        outerKeyPad[i] ^= 0x5c;
    }

    // perform inner SHA256
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, innerKeyPad, MAX_KEY_LENGTH);
    memcpy(buffer + MAX_KEY_LENGTH, data, dataLen);

    Sha256_ComputeHash(innerHash, buffer, MAX_KEY_LENGTH + dataLen);

    // perform outer SHA256
    memset(buffer, 0x00, sizeof(buffer));
    // cppcheck-suppress redundantCopy
    memcpy(buffer, outerKeyPad, MAX_KEY_LENGTH);
    memcpy(buffer + MAX_KEY_LENGTH, innerHash, SHA256_HASH_LENGTH);

    Sha256_ComputeHash(hash, buffer, MAX_KEY_LENGTH + SHA256_HASH_LENGTH);
}
