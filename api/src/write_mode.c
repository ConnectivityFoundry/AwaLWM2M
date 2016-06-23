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


#include <stddef.h>
#include <string.h>

#include "awa/server.h"
#include "write_mode.h"

bool IsWriteModeValid(AwaWriteMode writeMode)
{
    return ((writeMode >= AwaWriteMode_Replace) && (writeMode <= AwaWriteMode_Update));
}

// This table must align with awa/server.h:AwaWriteMode
static const char * WriteModeStrings[] =
{
    "AwaWriteMode_Replace",
    "AwaWriteMode_Update",
};

const char * WriteMode_ToString(AwaWriteMode writeMode)
{
    static const char * UnknownResult = "Unknown WriteMode";
    const char * result = UnknownResult;
    size_t numEntries = WriteMode_GetNumberOfWriteModeStrings();

    if ((writeMode >= 0) && (writeMode < numEntries))
    {
        result = WriteModeStrings[writeMode];
    }
    return result;
}

AwaWriteMode WriteMode_FromString(const char * writeModeString)
{
    AwaWriteMode i, writeMode = AwaWriteMode_LAST;
    for (i = (AwaWriteMode)0; i < AwaWriteMode_LAST; ++i)
    {
        if (strcmp(WriteModeStrings[i], writeModeString) == 0)
        {
            writeMode = i;
            break;
        }
    }
    return writeMode;
}

size_t WriteMode_GetNumberOfWriteModeStrings(void)
{
    return (sizeof(WriteModeStrings) / sizeof(WriteModeStrings[0]));
}

/**
 * @brief Get a descriptive name for the specified Write Mode
 * @param[in] writeMode Write Mode
 * @return pointer to an immutable c string
 */
const char * AwaWriteMode_ToString(AwaWriteMode writeMode)
{
    return WriteMode_ToString(writeMode);
}

/**
 * @brief Get a Write Mode from a descriptive name of a Write Mode
 * @param[in] writeMode Write Mode
 * @return pointer to an immutable c string
 */
AwaWriteMode AwaWriteMode_FromString(const char * writeModeString)
{
    return WriteMode_FromString(writeModeString);
}

