import nfqueue
import os
from scapy.all import *

server_ip = '192.168.0.101'
client_ip = '192.168.0.109'
port = '54321'
ACK_timer = 0

def setufw():
    scapy.config.conf.L3socket = L3RawSocket
    os.system('iptables -t raw -I PREROUTING -p tcp --dport ' + port + ' -j NFQUEUE')
    os.system('iptables -t mangle -I POSTROUTING -p tcp --sport ' + port + ' -j NFQUEUE')

def unsetufw():
    os.system('iptables -t raw -D PREROUTING -p tcp --dport ' + port + ' -j NFQUEUE')
    os.system('iptables -t mangle -D POSTROUTING -p tcp --sport ' + port + ' -j NFQUEUE')

def send_dummy_packet(src, dst, sport, dport, ipid, seq, ack):
    packet = IP(dst=dst, src=src, id=ipid, flags='DF')/TCP(sport=sport, dport=dport, seq=seq, ack=ack, flags='A')
    send(packet)

def process(i, payload):
    global ACK_timer
    data = payload.get_data()
    pkt = IP(data)
    
    # print pkt['IP'].src + ":" + str(pkt['TCP'].sport) + " ~ " + pkt['IP'].dst + ":" + str(pkt['TCP'].dport) 
    # print str(pkt['TCP'].seq) + " / " + str(pkt['TCP'].ack) 
    # print pkt['TCP'].flags
    print pkt.show()
    print "###############################################################"

    if pkt['TCP'].flags == 'A' and pkt['IP'].src == server_ip:
        ACK_timer = ACK_timer + 1
    else:
        ACK_timer = 0
        
    payload.set_verdict(nfqueue.NF_ACCEPT)

    if ACK_timer >= 4:
        send_dummy_packet(pkt['IP'].dst, pkt['IP'].src, pkt['TCP'].dport, pkt['TCP'].sport, 1001, pkt['TCP'].ack, pkt['TCP'].seq+1)
        ACK_timer = 0

def main():
    setufw()

    q = nfqueue.queue()
    q.open()
    q.bind(socket.AF_INET)
    q.set_callback(process)
    q.create_queue(0)

    try:
        q.try_run()
    except KeyboardInterrupt:
        print "Exiting..."
        q.unbind(socket.AF_INET)
        q.close()
        unsetufw()

if __name__ == '__main__':
    main()