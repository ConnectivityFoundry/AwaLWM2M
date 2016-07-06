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

#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

#include <sys/types.h>

namespace Awa {

// Spawn a new child process, arguments specified by a null-terminated commandVector.
// Note that the type of the vector must be char *, not const char *.
// If true, wait specifies that the parent should wait for the child to terminate.
// If true, silent specifies that the child's stdout will be redirected to /dev/null.
pid_t SpawnProcess(const std::vector<const char *> &commandVector, bool wait, bool silent);

// Kill an existing process, with SIGKILL.
void KillProcess(pid_t pid);

// Terminate an existing process, with SIGTERM.
void TerminateProcess(pid_t pid);

// Pause an existing process, with SIGSTOP.
void PauseProcess(pid_t pid);

// Unpause an existing process, with SIGCONT.
void UnpauseProcess(pid_t pid);

// Perform a CoAP operation on the specified resource, after a short delay.
pid_t CoAPOperation(const char * coapClientPath, int port, const char * method, const char * resource, int delay /*microseconds*/);


/**
 * @brief Check that the specified UDP port is not already in use.
 *        Note that this does not prevent the port allocation race condition.
 * @return true if port is in use, false if it is not.
 */
bool IsUDPPortInUse(int port);

// Send a request to the specified IPC port, wait for response. Return 0 on success, -1 on error or timeout
int WaitForIpc(int ipcPort, int timeout /*seconds*/, const char * request, size_t requestLen);

// Start an Awa Client process on the specified CoAP and IPC port. Redirect output to logFile. Return process ID, or 0 if failed.
pid_t StartAwaClient(const char * clientDaemonPath, int iCoapPort, int iIpcPort, const char * logFile, const char * endpointName, const char * bootstrapConfig, const char * bootstrapURI, const char * objectDefinitionsFile, const std::vector<std::string> & additionalOptions);

// Start an Awa Server process on the specified CoAP and IPC port. Redirect output to logFile. Return process ID.
pid_t StartAwaServer(const char * serverDaemonPath, int coapPort, int ipcPort, const char * logFile, const std::vector<std::string> & additionalOptions);

// Start an Awa Server process on the specified CoAP and IPC port. Redirect output to logFile. Return process ID.
pid_t StartAwaBootstrapServer(const char * bootstrapServerDaemonPath, int coapPort, const char * configFile, const char * logFile);

} // namespace Awa

#endif // PROCESS_H
