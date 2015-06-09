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


#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

#ifndef __GNUC__
#  define __attribute__(X)
#endif // __GNUC__

#define LOG_LEVEL_ERROR   AwaLogLevel_Error
#define LOG_LEVEL_WARNING AwaLogLevel_Warning
#define LOG_LEVEL_VERBOSE AwaLogLevel_Verbose
#define LOG_LEVEL_DEBUG   AwaLogLevel_Debug

#define LogError(...)   Log(LOG_LEVEL_ERROR,   stderr, __VA_ARGS__)
#define LogWarning(...) Log(LOG_LEVEL_WARNING, stderr, __VA_ARGS__)
#define LogVerbose(...) Log(LOG_LEVEL_VERBOSE, stdout, __VA_ARGS__)
#define LogDebug(...)   Log(LOG_LEVEL_DEBUG,   stdout, __VA_ARGS__)

#define LogNew(name, pointer)   LogDebug("New %s %p", name, pointer)
#define LogFree(name, pointer)  LogDebug("Free %s %p", name, pointer)

/**
 * @brief Print a log message with format + arguments (printf style)
 * @param[in] level Output if current level includes this level
 * @param[in] outFd Output stream (stderr, stdout)
 * @param[in] format Printf-style format string
 * @return The supplied error code
 */
#define Log(level, outFd, format, ...)                                   \
(                                                                        \
    _Log(level, outFd, "[%s:%d] " format "\n",                           \
         strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, \
         __LINE__,                                                       \
         ##__VA_ARGS__                                                   \
    )                                                                    \
)

/**
 * @brief Print a log message corresponding to the supplied error code, and
 *        print an optional message with format + arguments (printf style)
 * @param[in] errorCode Error code to display and return
 * @return The supplied error code
 */
#define LogErrorWithEnum(errorCode, ...) LogErrorWithEnum_(LOG_LEVEL_ERROR, stderr, errorCode, __VA_ARGS__)

// Avoid unused-value warning:
static inline AwaError ReturnError_(AwaError error) { return error; }

// Use an Expression to print the log message, then return the supplied error code
// (Note: declarations are not permitted inside an expression)
#define LogErrorWithEnum_(level, outFd, errorCode, format, ...)          \
(                                                                        \
    _Log(level, outFd, "[%s:%d] %s: " format "\n",                       \
         strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, \
         __LINE__,                                                       \
         Error_ToString(errorCode),                                      \
         ##__VA_ARGS__                                                   \
    ),                                                                   \
    ReturnError_(errorCode)  /* result of the expression */              \
)

#define LogPError(format, ...)                                           \
{ /* make a copy of the perror string for safety */                      \
    char * errorString = strerror(errno);                                \
    char * errorStringCopy = strdup(errorString);                        \
    _Log(LOG_LEVEL_ERROR, stderr, "[%s:%d] %s: " format "\n",            \
         strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, \
        __LINE__,                                                        \
        errorStringCopy,                                                 \
        ##__VA_ARGS__                                                    \
    );                                                                   \
    free(errorStringCopy);                                               \
}

void SetLogLevel(int level);

void _Log(int level, FILE * outFd, const char * format, ...) __attribute__ ((format (printf, 3, 4)));

void _vLog(int level, FILE * outFd, const char * format, va_list argp);

void LogHex(int level, FILE * outFd, const void * data, size_t length);

#ifdef __cplusplus
}
#endif

#endif // LOG_H

