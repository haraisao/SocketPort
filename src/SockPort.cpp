/*
 * socket.c
 *
 *      Copyright(c) Isao Hara, 2006
 *
 * $Revision: 1.3 $
 * $Date: 2008/05/29 04:54:56 $
 * $Id: socket.c,v 1.3 2008/05/29 04:54:56 yoshi Exp $
 *
 */

#include "SockPort.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *
 */
int32_t
copy_fds_set(fd_set *target, fd_set *src)
{
  int k;
  int	 nfds = 1;

  for(k = FD_SETSIZE-1; k >0 ;k--){
    if(FD_ISSET(k, src)){
      FD_SET(k, target);
      if(nfds < k) nfds = k+1;
    }
  }
  return nfds;
}

int subtract_time(struct timeval *tm1, struct timeval *tm2)
{
  int res = (tm2->tv_sec-tm1->tv_sec)*1000000 + (tm2->tv_usec-tm1->tv_usec);
  return res;
}

void sleep_onecycle(int timeout, struct timeval *tm1, struct timeval *tm2)
{
  int res = timeout*1000 - subtract_time(tm1, tm2);
  if (res>0){
    usleep(res);
  }
  return;
}

void
get_current_time(struct timeval *tv)
{
#ifdef Linux
  gettimeofday(tv, NULL); 
#else
  struct timezone tz;
  gettimeofday(tv, &tz); 
#endif
  return;
}

#ifdef __cplusplus
}
#endif

/*
 *
 */
SocketPort::SocketPort()
{
  sock_ = -1;
  service_func=NULL;
  idle_func=NULL;
  connect_func=NULL;
}


int
SocketPort::init_socket(int port, char* hostname)
{
  int reuse = 1;

  this->sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if(this->sock_ <= 0){
    std::cerr << "Fail to create socket" << std::endl;
    return -1;
  }
  setsockopt(this->sock_,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse, sizeof(reuse));

  memset(&this->addr_, 0, sizeof(struct sockaddr_in));
  this->addr_.sin_family=AF_INET;
  this->addr_.sin_port=htons(port);

  if(!hostname) {
    this->addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  }else{
    struct hostent *hp;
    if((hp = gethostbyname(hostname)) == NULL){
      std::cerr << "host: " << hostname << " not validt" << std::endl;
      return -1;
    }
    memcpy((char *)&(this->addr_.sin_addr),hp->h_addr_list[0],hp->h_length); 
  }
  return 1;

}

void
SocketPort::close()
{
  for(auto itr=this->services_.begin(); itr != this->services_.end(); itr++){
    (*itr)->close();
  }
  this->services_.clear();
  if(this->sock_>0){
    std::cerr << "Close socket "<< this->sock_ << std::endl;
    ::close(this->sock_);
    this->sock_=-1;
  }
  return;
}

SocketService *
SocketPort::find_service(int  sfd)
{
  for(auto itr=this->services_.begin(); itr != this->services_.end(); itr++){
    if((*itr)->get_port() == sfd) return (*itr);
  }
  return nullptr;
}

/*
 * Client side
 **/
int
SocketPort::connect(const char *hostname, int port)
{
  init_socket(port, (char *)hostname);
  if(::connect(this->sock_, (struct sockaddr *)&this->addr_,
			  sizeof(struct sockaddr_in)) < 0){
    return(-1);
  }
  return this->sock_;
}

/*
 *   Server Side
 */
int
SocketPort::listen(int port_no, int no) 
{
  init_socket(port_no);
  if (::bind(this->sock_, (struct sockaddr *)&this->addr_,
			  sizeof(struct sockaddr_in)) < 0){
    std::cerr << "Fail to bind a socket" << std::endl;
    return -1;
  }

  if (::listen (this->sock_, no) < 0 ){
    std::cerr << "Listen failed" << std::endl;
    return -1;
  }
  return this->sock_;
}

/*
 *
 */
int
SocketPort::accept() 
{
  struct sockaddr_in  client_addr;
  int   client_len, snew, opt_len;
  int32_t  opt;

  client_len = sizeof(struct sockaddr_in);
  snew = ::accept(this->sock_, (struct sockaddr *)&client_addr, (socklen_t*)&client_len);
 
  if (snew>0) {
    getsockopt(snew, SOL_SOCKET, SO_REUSEADDR, &opt, (socklen_t*)&opt_len);
    opt = 1;
    setsockopt(snew, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    getsockopt(snew, SOL_SOCKET, SO_REUSEADDR, &opt, (socklen_t*)&opt_len);
  }

  return(snew);
}

/*
 *
 */
int
SocketPort::select_sockets()
{
  int	 nfds;
  fd_set socks;
  int	 stat, i = 0, newsock;

  FD_ZERO(&socks);
  nfds = copy_fds_set(&socks, &this->sockbits_);

  stat=select(nfds, &socks, 0, 0/* &exceptions */, &this->time_out_);
  if (stat>0) {
    if (FD_ISSET(this->sock_, &socks)) { /* New connection request */
      newsock = this->accept(); /* create new connection */
      if (newsock>0) FD_SET(newsock, &this->sockbits_);
      FD_CLR(this->sock_, &socks);

      this->connection_request(newsock);
      stat--;
    } else { /* service request called */
      i=0;
      while (stat > 0 && i < nfds) {
        if (FD_ISSET(i, &socks)) {
          //std::cout << "=== Request!! (" << i << ")" << std::endl;
	  SocketService *service_=this->find_service(i);
	  int res = -1;
	  if(service_ ){
	    res=service_->execute();  
	  }
	  if (res<0){
	    if(service_){
	      this->services_.remove(service_);
	      delete service_;
	    }
            //std::cout << "=== Close!! (" << i << ")" << std::endl;
	    FD_CLR(i, &this->sockbits_);
	  }
          FD_CLR(i, &socks);
          stat--;
        }
      i++; 
      }
    }
  }
  return i; 
}

void
SocketPort::set_timeout(float msec)
{
  this->time_out_.tv_sec = (unsigned int)(msec / 1000.0);
  this->time_out_.tv_usec = (msec - this->time_out_.tv_sec * 1000) * 1000 ;
  return;
}

void
SocketPort::idle()
{
  if(this->idle_func){ this->idle_func(this); }
  return;
}

void
SocketPort::connection_request(int sfd)
{
  SocketService *service_ = new SocketService(sfd);
  service_->set_service_function(this->service_func);
  this->services_.push_back(service_);
  if(this->connect_func){
    this->connect_func(this);
  }
  return;
}

/*
 *
 */
void
SocketPort::server_loop(float time_out_msec)
{
  struct timeval time1,time2;

  FD_ZERO(&this->sockbits_);
  FD_SET(this->sock_, &this->sockbits_);
  this->set_timeout(time_out_msec);

  while (1) {
      get_current_time(&time1); 
      select_sockets();

      this->idle();

      get_current_time(&time2); 
      sleep_onecycle(time_out_msec, &time1, &time2);
  }
  return;
}

/*
 *
 */
int
SocketPort::send_msg(std::string msg)
{
  int res;
  res = write(this->sock_, msg.c_str(), msg.length());
  return res;
}

std::string
SocketPort::recieve_msg()
{
  char recvbuf[MSG_LEN];
  int s=read(this->sock_, recvbuf, MSG_LEN);
  if(s<=0){
    return nullptr;
  }
  return std::string(recvbuf); 
}

void
SocketPort::load_parameter(const char *file)
{
  this->config_ = YAML::LoadFile(file);
  return;
}

std::string
SocketPort::get_param_string(const char *name, std::string val)
{
  return ::get_param_string(this->config_, name, val);
}

int
SocketPort::get_param_int(const char *name, int val)
{
  return ::get_param_int(this->config_, name, val);
}

/***** SocketService ****/
SocketService::SocketService(int sfd)
{
  this->set_port(sfd);
  this->set_service_function(NULL);
}

int
SocketService::execute()
{
  int res=-1;
  //std::cout << "Call execute:" << this->get_port() << std::endl;
  if(this->service_func){
    res=this->service_func(this);
  }
  return res;
}

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *   find a hostname and a port number from a socket descriptor
 */
short
get_socket_port(int sock)
{
  int len;
  struct sockaddr_in addr;

  len = sizeof(struct sockaddr_in);

  if (getsockname(sock, (struct sockaddr *)&addr, (socklen_t*)&len) < 0) return -1;
  return  ntohs(addr.sin_port);
}

#if defined(Linux) || defined(Cygwin)
char *
get_ip_address(int sock)
{
  struct sockaddr_in *addr=0;
  struct ifreq ifreqs[100];
  struct ifconf ifconf;
  int i, num;
  
  ifconf.ifc_len = sizeof(ifreqs);
  ifconf.ifc_req = ifreqs;

  if(ioctl(sock, SIOCGIFCONF, &ifconf) < 0)
  {
    return (char *)NULL;
  }
  num = ifconf.ifc_len / sizeof(struct ifreq);

  for(i=0; i< num ; i++){
    addr = (struct sockaddr_in *)&ifreqs[i].ifr_ifru.ifru_addr;
    char *ip_addr = (char *)inet_ntoa(addr->sin_addr);

    if( (addr->sin_family == AF_INET)
        && (strcmp(ip_addr,"127.0.0.1") != 0)
        && (strncmp(ip_addr,"169.254.", 8) != 0)
        && (strcmp(ip_addr,"0.0.0.0") != 0)
        )
      {
           return strdup(ip_addr);
      }
  }
  return (char *)NULL;
}
#else
char *
get_ip_address(int sock)
{
  int stat;
  struct ifaddrs *ifap;
  char * res = NULL;
  struct sockaddr *addr=NULL;
  struct sockaddr_in *addr2=NULL;

  stat = getifaddrs(&ifap); 

  if(stat == 0){
    while(ifap != NULL){
      addr = ifap->ifa_addr;
      addr2 = (struct sockaddr_in *)addr;
      char *ip_addr = (char *)inet_ntoa(addr2->sin_addr);

      if( (addr->sa_family == AF_INET) &&
        (strncmp(ip_addr, "127.0.0", 7) != 0) &&
        (strncmp(ip_addr, "172.", 4) != 0) &&
        (strcmp(ip_addr, "0.0.0.0") != 0)
      ){
        res = strdup(ip_addr);
        break;
      }else{
        ifap = ifap->ifa_next;
      }
    }
  }else{
    std::cerr << "ERROR: fail to getifaddrs" << std::endl;
  }
  return res;
}
#endif

#ifdef __cplusplus
}
#endif
