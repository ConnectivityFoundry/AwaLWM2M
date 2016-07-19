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

#include <vector>
#include <string>
#include <algorithm>

#include "support.h"

namespace Awa {

namespace global {

    // non-const initialization values are subject to Static Initialization Order Fiasco
    int logLevel = 0;

    // initialise non-const globals with a function to avoid Static Initialization Order Fiasco
    void SetGlobalDefaults(void)
    {
        global::logLevel = defaults::logLevel;
        global::timeout = defaults::timeout;
        SetDaemonGlobalDefaults();
    }

} // namespace global

namespace detail {
    const char * NonRoutableIPv4Address = "192.0.2.0";
    const char * NonRoutableIPv6Address = "2001:db8::";
} // namespace detail

bool ElapsedTimeWithinTolerance(double time_ms, double time_target_ms, double tolerance_ms)
{
    // returns true if the actual time was within the target time +/- tolerance.
    return (time_ms >= time_target_ms - tolerance_ms) && (time_ms <= time_target_ms + tolerance_ms);
}

bool ElapsedTimeExceeds(double time_ms, double time_target_ms)
{
    // returns true if the actual time was equal or greater than the target time.
    return (time_ms >= time_target_ms);
}

struct RegistrationCheck {
    RegistrationCheck() : registeredIDs() {}
    static void Callback(const AwaServerClientRegisterEvent * event, void * context)
    {
//        std::cerr << "Callback" << std::endl;
        RegistrationCheck * chk = static_cast<RegistrationCheck *>(context);
        chk->HandleEvent(event);
    }
    void HandleEvent(const AwaServerClientRegisterEvent * event) {
        AwaClientIterator * clientIterator = AwaServerClientRegisterEvent_NewClientIterator(event);
        while (AwaClientIterator_Next(clientIterator))
        {
            std::string clientID(AwaClientIterator_GetClientID(clientIterator));
//            std::cerr << "Client: " << clientID << std::endl;
            registeredIDs.push_back(clientID);
        }
        AwaClientIterator_Free(&clientIterator);
    }

    std::vector<std::string> registeredIDs;
};

/**
 * @brief Wait for clients specified by clientIDs to register with Server, within a timeout period.
 * @return true if all clients register within timeout period.
 * @return false if all clients do not register after timeout period has expired.
 */
bool WaitForRegistration(AwaServerSession * serverSession, const std::vector<std::string> & clientIDs, int timeoutMs)
{
    useconds_t timeout = timeoutMs * 1000;
    struct RegistrationCheck chk;
    AwaServerSession_SetClientRegisterEventCallback(serverSession, RegistrationCheck::Callback, &chk);

    // assume no clients will deregister!
    bool timedOut = false;
    bool allRegistered = false;

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

//    std::cerr << "WaitForRegistration:" << std::endl;
    do
    {
        AwaServerSession_Process(serverSession, global::timeout);
        AwaServerSession_DispatchCallbacks(serverSession);

        // short sleep to avoid spinning the IPC too fast
        usleep(10000);

        allRegistered = (chk.registeredIDs.size() == clientIDs.size()) && std::is_permutation(chk.registeredIDs.begin(), chk.registeredIDs.end(), clientIDs.begin());

        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);

        struct timeval elapsedTime;
        timersub(&currentTime, &startTime, &elapsedTime);
        useconds_t elapsed = elapsedTime.tv_sec * 1e6 + elapsedTime.tv_usec;

        timedOut = !allRegistered && (elapsed > timeout);
//        std::cerr << "elapsed: " << elapsed << std::endl;
//        std::cerr << "allRegistered " << allRegistered << std::endl;
//        std::cerr << "timedOut " << timedOut << std::endl;

    } while (!allRegistered && !timedOut);

    AwaServerSession_SetClientRegisterEventCallback(serverSession, NULL, NULL);

    return allRegistered;
}

} // namespace Awa
