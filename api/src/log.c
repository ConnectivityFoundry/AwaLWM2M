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


#include <stdarg.h>

#include "log.h"

static int g_logLevel = 0;

AwaLogLevel AwaLog_SetLevel(AwaLogLevel level)
{
    AwaLogLevel oldLevel = g_logLevel;
    SetLogLevel(level);
    return oldLevel;
}

void SetLogLevel(int level)
{
    g_logLevel = level;
}

void _Log(int level, FILE * outFile, const char * format, ...)
{
    if (g_logLevel >= level)
    {
        va_list argp;
        outFile = (outFile == NULL) ? stdout : outFile;
        va_start(argp, format);
        _vLog(level, outFile, format, argp);
        va_end(argp);
        fflush(outFile);
    }
}

void _vLog(int level, FILE * outFile, const char * format, va_list argp)
{
    if (g_logLevel >= level)
    {
        outFile = (outFile == NULL) ? stdout : outFile;
        vfprintf(outFile, format, argp);
        fflush(outFile);
    }
}

void LogHex(int level, FILE * outFd, const void * data, size_t length)
{
    const uint8_t * data_ = (const uint8_t *)data;
    size_t i = 0;
    for (i = 0; i < length; ++i)
    {
        _Log(level, outFd, "%02x ", data_[i]);
        if (i % 16 == 15)
        {
            _Log(level, outFd, "\n");
        }
    }
    if (length % 16 != 0)
    {
        _Log(level, outFd, "\n");
    }
}

