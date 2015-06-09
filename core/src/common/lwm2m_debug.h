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


#ifndef LWM2M_DEBUG_H
#define LWM2M_DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DebugLevel_Emerg=0,
    DebugLevel_Alert,
    DebugLevel_Crit,
    DebugLevel_Error,
    DebugLevel_Warning,
    DebugLevel_Notice,
    DebugLevel_Info,
    DebugLevel_Debug
} DebugLevel;

#define DEBUG_FILELINE
#define DEBUG_FANCY

#define xstringy(a) stringy(a)
#define stringy(a) #a

// Host dependent directory slash
#define DIR_SLASH '/'

#ifndef __GNUC__
#  define __attribute__(X)
#endif // __GNUC__

void Lwm2m_Printf(DebugLevel level, char const * format, ...) __attribute__ ((format (printf, 2, 3)));

#ifdef DEBUG_FANCY
#define ANSI_COLOUR_RED     "\x1b[31m"
#define ANSI_COLOUR_GREEN   "\x1b[32m"
#define ANSI_COLOUR_YELLOW  "\x1b[33m"
#define ANSI_COLOUR_BLUE    "\x1b[34m"
#define ANSI_COLOUR_MAGENTA "\x1b[35m"
#define ANSI_COLOUR_CYAN    "\x1b[36m"
#define ANSI_COLOUR_RESET   "\x1b[0m"
#else
#define ANSI_COLOUR_RED
#define ANSI_COLOUR_GREEN
#define ANSI_COLOUR_YELLOW
#define ANSI_COLOUR_BLUE
#define ANSI_COLOUR_MAGENTA
#define ANSI_COLOUR_CYAN
#define ANSI_COLOUR_RESET
#endif

#ifdef DEBUG_FILELINE

void Lwm2m_PrintBanner(void);

#define _Lwm2m_Printf(COLOUR, level, format, ...) \
{\
    const char * fileName = __FILE__;\
    const char * lineNum = xstringy(__LINE__);\
    const char * shortFileName = strrchr(fileName, DIR_SLASH);\
    Lwm2m_Printf(level, ANSI_COLOUR_YELLOW "[%s:%s] " COLOUR format ANSI_COLOUR_RESET, shortFileName ? shortFileName+1 : fileName, lineNum, ##__VA_ARGS__);\
}\

#else

#define _Lwm2m_Printf(level, format, ...) \
        Lwm2m_Printf(, level, format, ##__VA_ARGS__)
#endif

#define Lwm2m_Debug(format, ...) \
    _Lwm2m_Printf(ANSI_COLOUR_RESET, DebugLevel_Debug, format, ##__VA_ARGS__)

#define Lwm2m_Error(format, ...) \
    _Lwm2m_Printf(ANSI_COLOUR_RED, DebugLevel_Error, format, ##__VA_ARGS__)

#define Lwm2m_Warning(format, ...) \
    _Lwm2m_Printf(ANSI_COLOUR_YELLOW, DebugLevel_Warning, format, ##__VA_ARGS__)

#define Lwm2m_Info(format, ...) \
    _Lwm2m_Printf(ANSI_COLOUR_CYAN, DebugLevel_Info, format, ##__VA_ARGS__)

void Lwm2m_SetOutput(FILE * output);

void Lwm2m_SetLogLevel(DebugLevel level);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_DEBUG_H
