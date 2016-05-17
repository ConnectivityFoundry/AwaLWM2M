#/************************************************************************************************************************
# Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
#        following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#        following disclaimer in the documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
#        products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#************************************************************************************************************************/

#!/usr/bin/env python
"""Send XML file (or stdin) to LWM2M IPC."""

import argparse
import sys
import socket

g_description = "Send XML via stream to specified IPC channel and print response."
g_epilog = """Examples:
  ipc.py get.xml                      # Send XML from file
  ipc.py                              # Enter/paste XML and hit CTRL-D to send
  echo "<xml..." | ipc.py             # Pipe XML in
  ipc.py --ipc udp://127.0.0.1:54321  # Specify IPC channel
"""

g_DEBUG = False

def main():

    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description=g_description,
                                     epilog=g_epilog)
    parser.add_argument('infile', nargs='?', type=argparse.FileType('r'), help="Input file (blank for stdin).", default=sys.stdin)
    parser.add_argument('outfile', nargs='?', type=argparse.FileType('w'), help="Output file (blank for stdout)", default=sys.stdout)
    parser.add_argument('-i', '--ipc', nargs='?', help="Specify IPC channel.", default="udp://127.0.0.1:12345")

    args = parser.parse_args()

    # read entire infile
    request = args.infile.read()
    channel = args.ipc

    response = send_request_and_receive_response(channel, request)
    print(response)

def send_request_and_receive_response(channel, request):
    protocol, ipc_id = channel.split("://")
    if protocol == 'udp':
        response = udp_ipc(ipc_id, request)
    else:
        raise Exception("Invalid IPC protocol");
    return response

def receive_datagram(channel):
    protocol, ipc_id = channel.split("://")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    data, addr = sock.recvfrom(65536)
    return data

def udp_ipc(id, request):

    address, port = id.split(":")
    port = int(port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # send UDP packet
    if g_DEBUG: print("IPC SEND:\n" + request)
    sock.sendto(request, (address, port))

    # retrieve and print response
    data, addr = sock.recvfrom(65536)
    if g_DEBUG: print("IPC RECV:\n" + data)
    return data

if __name__ == "__main__":
    main()
