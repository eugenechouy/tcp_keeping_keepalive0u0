import socket, sys,time
from scapy.all import *

def set_keepalive_linux(sock, after_idle_sec=1, interval_sec=3, max_fails=5):
    """Set TCP keepalive on an open socket.

    It activates after 1 second (after_idle_sec) of idleness,
    then sends a keepalive ping once every 3 seconds (interval_sec),
    and closes the connection after 5 failed ping (max_fails), or 15 seconds
    """

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
    # every *after_idle_sec send heartbeat
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, after_idle_sec)
    # if not response heartbeat send every *interval_sec *max_fails time
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, interval_sec)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, max_fails)

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, msg:
    sys.stderr.write("[ERROR] %s\n" % msg[1])
    sys.exit(1)

try:
    sock.connect(('172.20.10.9', 54321))
except socket.error, msg:
    sys.stderr.write("[ERROR] %s\n" % msg[1])
    exit(1)

a = 0

while True:
    time.sleep(5)
    sock.send("Hello I'm Client"+str(a)+".\r\n")
    a += 1
    try:
        msg = sock.recv(1024)
        print msg
    except socket.error as e:
        print e

sock.close()

