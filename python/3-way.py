import sys
import socket
from scapy.all import *

server_ip = '192.168.0.101'
client_ip = '192.168.0.109'
port = 54321

# since scapy create its own packet and bypass the whole TCP/IP stack. The kernel doesn't
# know about it and will sen RST as response
os.system('iptables -A OUTPUT -p tcp --tcp-flags RST RST -s ' + client_ip + ' -j DROP')

scapy.config.conf.L3socket = L3RawSocket

sport = random.randint(1024,65535)

ip = IP(dst=server_ip)
SYN = TCP(sport=sport, dport=port, flags='S', seq=1000)
SYNACK = sr1(ip/SYN)

ACK = TCP(sport=sport, dport=port, flags='A', seq=SYNACK.ack + 1, ack=SYNACK.seq+1)
send(ip/ACK)

os.system('iptables -D OUTPUT -p tcp --tcp-flags RST RST -s ' + client_ip + ' -j DROP')