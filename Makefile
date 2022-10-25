#
OBJS=src/param.o src/SockPort.o
SRCS=$(OBJS: .o=.cpp)
CFLAGS= -DLinux -I./include -I../yaml-cpp/include
LFLAGS= -L./lib -L../yaml-cpp/lib
CPP=g++
LIBS= -lSockPort -lyaml-cpp

LIBNAME=lib/libSockPort.a
EXAMPLES=bin/server bin/client
EXAMPLE_OBJS=examples/server.o examples/client.o
EXAMPLE_SRCS=$(EXAMPLE_OBJS: .o=.cpp)

all: $(LIBNAME) $(EXAMPLES)

$(LIBNAME): $(OBJS)
	ar rc $(LIBNAME) $(OBJS)
	ranlib $(LIBNAME)

.cpp.o: 
	$(CPP) -c $(CFLAGS) $< -o $@

bin/server: examples/server.o
	$(CPP) $(LFLAGS) -o $@  $< $(LIBS)

bin/client: examples/client.o
	$(CPP) $(LFLAGS) -o $@  $< $(LIBS)

clean:
	rm $(LIBNAME) $(OBJS) $(EXAMPLES) $(EXAMPLE_OBJS)
