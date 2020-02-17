.PHONY: all

SRC=detect_tcp.c tcp_send.c
INCL=-I/usr/local/libnetfilter_queue-1.0.3/include
LIB=-L/usr/local/libnetfilter_queue-1.0.3/lib -lnetfilter_queue -lmnl
DPORT=54321

build:
	gcc -o detect_tcp $(INCL) $(SRC) $(LIB)

run:
	iptables -t raw -I PREROUTING -p tcp --dport $(DPORT) -j NFQUEUE
	iptables -t mangle -I POSTROUTING -p tcp --sport $(DPORT) -j NFQUEUE
	export LD_LIBRARY_PATH=/usr/local/libnetfilter_queue-1.0.3/lib
	./detect_tcp

clean:
	rm -f detect_tcp
	iptables -t raw -D PREROUTING -p tcp --dport $(DPORT) -j NFQUEUE
	iptables -t mangle -D POSTROUTING -p tcp --sport $(DPORT) -j NFQUEUE