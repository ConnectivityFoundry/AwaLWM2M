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
#include <stdarg.h>

#include "lwm2m_debug.h"

static DebugLevel debugLevel = DebugLevel_Info;
static FILE * g_outFile = NULL;

void Lwm2m_PrintBanner(void)
{
    const char * banner =
    "   _____\n"
    "  / ___  \\_______    /|                                     _\n"
    " / /    |  ____  |  | | _ __ ___   __ _  __ _ _ _ __   __ _| |_ _  ___  _ __\n"
    " \\ \\____| |    | |  | || '_ ` _ \\ / _` |/ _` | | '_ \\ / _` | __| |/ _ \\| '_ \\ \n"
    "  \\____,| |____| |  | || | | | | | (_| | (_| | | | | | (_| | |_| | (_) | | | |\n"
    "        |________|  |_||_| |_| |_|\\__,_|\\__, |_|_| |_|\\__,_|\\__|_|\\___/|_| |_|\n"
    "                                         __/ |\n"
    "                                        |___/        Technologies\n";
    Lwm2m_Printf(0, "\n\n" ANSI_COLOUR_MAGENTA "%s" ANSI_COLOUR_RESET "\n", banner);
}

void Lwm2m_Printf(DebugLevel level, char const * format, ...)
{
     if (level <= debugLevel)
     {
         va_list args;

         if (g_outFile == NULL)
             g_outFile = stdout;

         va_start(args, format);
         vfprintf(g_outFile, format, args);
         va_end(args);

         fflush(g_outFile);
     }
}

void Lwm2m_SetOutput(FILE * outFile)
{
    g_outFile = outFile;
}

void Lwm2m_SetLogLevel(DebugLevel level)
{
    debugLevel = level;
}
