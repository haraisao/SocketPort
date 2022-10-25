/*


 */
#pragma once
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <functional>

#include <params.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE	32
#endif

#ifndef NFDBITS
#define NFDBITS		32
#endif

#define DEFAULT_HOST	"localhost"
#define DEFAULT_PORT	11000

#define MSG_LEN 4096

/*
 */
#ifdef __cplusplus
extern "C"
{
#endif
void get_current_time(struct timeval *tv);
int subtract_time(struct timeval *tv1, struct timeval *tv2);
void sleep_onecycle(int timeout, struct timeval *tv1, struct timeval *tv2);
short get_socket_port(int sock);
char * get_ip_address(int sock);

#ifdef __cplusplus
}
#endif

class SocketService;
/*
 *
 */
class SocketPort 
{
public:
  SocketPort();

  virtual ~SocketPort(){ close(); }

  int init_socket(int port, char *hostname=NULL);
  int listen(int port, int no=5);
  int connect(const char *hostname, int port);
  int connect(std::string hostname, int port){
    return connect(hostname.c_str(), port);
  }

  void server_loop(float time_out_msec);

  //std::string read_line();

  int get_port() { return sock_; }
  void set_port(int fd) { sock_ = fd; }
  void close();

  bool is_activate() { return (sock_ > 0); } 
  void set_timeout(float msec);

  int send_msg(std::string msg);
  std::string recieve_msg();

  SocketService *find_service(int sfd);

  void load_parameter(const char *file);
  std::string get_param_string(const char *name, std::string val);
  int get_param_int(const char *name, int val);

  virtual void idle();
  virtual void connection_request(int sfd);

  void set_service_function(int (*func)(SocketService *)){
    service_func=func; 
  }

  void set_connect_function(int (*func)(SocketPort *)){
    connect_func=func; 
  }

  void set_idle_function(void (*func)(SocketPort *)){
    idle_func=func; 
  }


private:
  int accept();
  int select_sockets();

public:
  int (*service_func)(SocketService *);
  void (*idle_func)(SocketPort *);
  int (*connect_func)(SocketPort *);

private:
  int sock_;
  int port_;
  struct sockaddr_in addr_;

  struct timeval time_out_;
  fd_set sockbits_;
  std::stringstream msgbuf_;

  typedef void*(*func)(void *);
  std::map<std::string, func> func_list_;

  std::list<SocketService *> services_;
  YAML::Node config_;

};


/*
 *
 */
class SocketService: public SocketPort
{
public:
  SocketService(int sfd);

  virtual int execute();

private:

};	
