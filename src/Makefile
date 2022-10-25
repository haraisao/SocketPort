#
OBJS=param.o SockPort.o
SRCS=$(OBJS: .o=.cpp)
CFLAGS= -DLinux -I../include -I../../yaml-cpp/include
CPP=g++

NAME=libSockPort.a

$(NAME): $(OBJS)
	ar rc $(NAME) $(OBJS)
	ranlib $(NAME)

all: $(NAME)

.cpp.o: 
	$(CPP) -c $(CFLAGS) $< -o $@


clean:
	rm $(NAME) $(OBJS)
