/*
 */
#include <SockPort.h>

#define COM_PORT 11000

std::string hostname="localhost";
int port=COM_PORT;

int
main(int argc, char **argv)
{
  std::string msgbuf;
  char recvbuf[MSG_LEN];
  ssize_t s;
  char *param_file="param.yaml";
  SocketPort *sock;

  if (argc>1){
     param_file=argv[1];
  }

  sock = new SocketPort();
  sock->load_parameter(param_file);
  hostname=sock->get_param_string("config/host", hostname);
  port=sock->get_param_int( "config/port", COM_PORT);
  sock->connect(hostname, port);

  while (sock->is_activate())
  {
    std::cout << "===> ";
    if(!std::getline(std::cin, msgbuf) || msgbuf.length() == 0 ){
      if(std::cin.eof()){ break; }
      continue;
    }

    if(msgbuf == "bye"){
      s=sock->send_msg(msgbuf);
      break;
    }else{
      s=sock->send_msg(msgbuf);
      std::cout << "== Send:" << msgbuf << ": " << s << std::endl;
      msgbuf=sock->recieve_msg();
      std::cout << "==Recv:" << recvbuf << ":" << s << std::endl;
      if (msgbuf.length() <= 0){ break; }
    }
  }
  delete sock;

  std::cout << "=== Terminated ====" << std::endl;
}
