#
OBJS=param.o SockPort.o
SRCS=$(OBJS: .o=.cpp)
CFLAGS= -DLinux -I../include -I../../yaml-cpp/include 
LFLAGS= -L../src -L../../yaml-cpp/lib
CPP=g++
#LIBS=libSockPort.a
LIBS= -lSockPort -lyaml-cpp


all: server client

.cpp.o: 
	$(CPP) -c $(CFLAGS) $< -o $@

server: server.o
	$(CPP) $(LFLAGS) -o $@  server.o $(LIBS)

client: client.o
	$(CPP) $(LFLAGS) -o $@  client.o $(LIBS)

clean:
	rm $(NAME) $(OBJS)
