#ifndef MINIFTP_TCP_CLIENT_
#define MINIFTP_TCP_CLIENT_

#include <WinSock2.h>

#include <string>
#include <fstream>

namespace miniftp {

typedef char Byte;
const long long MAX_INT = 0x7FFFFFFF;

enum DataType {
  SHORT,
  LONG,
  LONG_LONG,
  FLOAT,
  DOUBLE
};

class TCPClient {
 public:
  TCPClient();
  TCPClient(const std::string& ip_addr, const int& port);
  ~TCPClient();

 public:
  const WSADATA& wsa_data();
  const SOCKET& socket_client();
  const SOCKADDR_IN& server_addr();
  int port();
  bool is_connected();

 public:
  bool Conn();
  bool Conn(const std::string& ip_addr, const int& port);
  void Close();
  bool Send(const Byte* data, long long length);
  bool Recv(char* buffer, long long length);

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
