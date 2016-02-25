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

def udp_ipc(id, request):

    address, port = id.split(":")
    port = int(port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # send UDP packet
    sock.sendto(request, (address, port))

    # retrieve and print response
    data, addr = sock.recvfrom(65536)
    return data

if __name__ == "__main__":
    main()
