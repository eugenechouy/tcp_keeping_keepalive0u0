import socket, sys, select
from thread import *

def set_keepalive_linux(sock, after_idle_sec=1, interval_sec=3, max_fails=5):
    """Set TCP keepalive on an open socket.

    It activates after 1 second (after_idle_sec) of idleness,
    then sends a keepalive ping once every 3 seconds (interval_sec),
    and closes the connection after 5 failed ping (max_fails), or 15 seconds
    """
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, after_idle_sec)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, interval_sec)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, max_fails)

def threadWork(client):
    while True:
        try:
            msg = client.recv(1024)
            if not msg:
                pass
            else:
                print "Client send: " + msg 
                client.send("You say: " + msg + "\r\n")
        except socket.error as e:
            print e
    client.close()

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, msg:
    sys.stderr.write("[ERROR] %s\n" % msg[1])
    sys.exit(1)

sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', 54321))
sock.listen(5)

while True: 
    (csock, adr) = sock.accept()
    set_keepalive_linux(csock, 10, 1, 5)
    print "Client Info: ", csock, adr 
    start_new_thread(threadWork, (csock,))

sock.close()