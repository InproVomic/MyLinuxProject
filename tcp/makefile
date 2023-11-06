.PHONY:all
all:tcpserver tcpclient

tcpserver:tcpserver.cc
	g++ -o $@ $^ -std=c++11 -lpthread

tcpclient:tcpclient.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -f tcpserver tcpclient