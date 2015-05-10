#ifndef MINIFTP_TCP_CLIENT_
#define MINIFTP_TCP_CLIENT_

#include <WinSock2.h>

#include <string>
#include <fstream>
#include <list>
#include <utility>

namespace miniftp {

typedef char Byte;
const int MAX_INT = 0x7FFFFFFF;

class TCPClient {
 public:
  TCPClient();
  TCPClient(const std::string& ip_addr, const int& port);
  ~TCPClient();

 public:
  bool Conn();
  bool Conn(const std::string& ip_addr, const int& port);
  void Close();
  bool Send(const Byte* data, long long length);
  bool Send(const std::list< std::pair<Byte*, int> >& blocks, long long length);
  bool Recv(char* buffer, long long length);
  bool Recv(const std::list< std::pair<Byte*, int> >& blocks, long long length);

 private:
  WSADATA wsa_data_;
  SOCKET socket_client_;
  SOCKADDR_IN server_addr_;

  std::string host_name_;
  int port_;
  bool is_connected_;
};

}
#endif
