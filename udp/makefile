.PHONY:all
all:udpserver udpclient

udpserver:udpserver.cc
	g++ -o $@ $^ -std=c++11 -lpthread

udpclient:udpclient.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -f udpserver udpclient
