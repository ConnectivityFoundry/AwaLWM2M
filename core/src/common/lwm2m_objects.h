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


#ifndef LWM2M_OBJECTS_H
#define LWM2M_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define LWM2M_SECURITY_OBJECT                          0
#  define LWM2M_SECURITY_OBJECT_SERVER_URI             0
#  define LWM2M_SECURITY_OBJECT_BOOTSTRAP_SERVER       1
#  define LWM2M_SECURITY_OBJECT_SECURITY_MODE          2
#  define LWM2M_SECURITY_OBJECT_PUBLIC_KEY             3
#  define LWM2M_SECURITY_OBJECT_SERVER_PUBLIC_KEY      4
#  define LWM2M_SECURITY_OBJECT_SECRET_KEY             5
#  define LWM2M_SECURITY_OBJECT_SMS_MODE               6
#  define LWM2M_SECURITY_OBJECT_SMS_BINDING_PARAM      7
#  define LWM2M_SECURITY_OBJECT_SMS_BINDING_SECRET_KEY 8
#  define LWM2M_SECURITY_OBJECT_SMS_NUMBER             9
#  define LWM2M_SECURITY_OBJECT_SHORT_SERVER_ID        10
#  define LWM2M_SECURITY_OBJECT_CLIENT_HOLD_OFF        11

#define LWM2M_SERVER_OBJECT                                 1
#  define LWM2M_SERVER_OBJECT_SHORT_SERVER_ID               0
#  define LWM2M_SERVER_OBJECT_LIFETIME                      1
#  define LWM2M_SERVER_OBJECT_MINIMUM_PERIOD                2
#  define LWM2M_SERVER_OBJECT_MAXIMUM_PERIOD                3
#  define LWM2M_SERVER_OBJECT_DEFAULT_MINIMUM_PERIOD        2
#  define LWM2M_SERVER_OBJECT_DEFAULT_MAXIMUM_PERIOD        3
#  define LWM2M_SERVER_OBJECT_DISABLE_TIMEOUT               5
#  define LWM2M_SERVER_OBJECT_NOTIFICATION_STORING          6
#  define LWM2M_SERVER_OBJECT_BINDING                       7

#define LWM2M_ACL_OBJECT                   2

#define LWM2M_DEVICE_OBJECT                3

#define LWM2M_CONNECTIVITY_MONITORING_OBJECT                    4

#define LWM2M_FIRMWARE_OBJECT                                   5

#define LWM2M_LOCATION_OBJECT                                   6

#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT                    7
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_SMS_TX_COUNTER     0
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_SMS_RX_COUNTER     1
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_TX_DATA            2
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_RX_DATA            3
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_MAX_MESSAGE_SIZE   4
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_AVG_MESSAGE_SIZE   5
#define LWM2M_CONNECTIVITY_STATISTICS_OBJECT_START_RESET        6

#ifdef __cplusplus
}
#endif

#endif // LWM2M_OBJECTS_H

