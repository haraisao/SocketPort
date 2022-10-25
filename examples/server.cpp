/*
 */
#include <SockPort.h>

#define COM_PORT 11000

/**** Global variables ****/
std::string hostname="localhost";
int port=COM_PORT;

/**** Functions ****/

int
cmd_req(SocketService *srv)
{
  char rcv_buf[MSG_LEN];
  memset(rcv_buf, 0, MSG_LEN);

  int sock=srv->get_port();
  ssize_t s=read(sock, rcv_buf, MSG_LEN);

  if(s < 0 || !strncmp(rcv_buf, "bye", 3)){
    close(sock);
    std::cout << "=== Terminated ===" << std::endl;
    return -1;
  }else{
    std::string msg(rcv_buf);
    std::string buf;

    std::cout << "=== Recv: " << msg << ":" <<msg.length() << std::endl;
    buf="OK";
    s=write(sock, buf.c_str(), buf.length());
    std::cout << "=== Send: " << buf << ":" << s << std::endl;
  }
  return 1;
}

int
connect_req(SocketPort *sock)
{
  std::cerr << "== Connect Req ====" << std::endl;
  return 1;
}

/*
 *
 */
int
main(int argc, char**argv)
{
  SocketPort *sock;
  char *param_file="param.yaml";

  if (argc > 1){
    param_file=argv[1];
  }

  sock = new SocketPort();
  sock->load_parameter(param_file);
  port=sock->get_param_int("config/port", COM_PORT);

  sock->set_service_function(cmd_req);
  sock->set_connect_function(connect_req);

  sock->listen(port);
  sock->server_loop(1.0);
}
