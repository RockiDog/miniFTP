#ifndef MINIFTP_FTP_CLIENT_
#define MINIFTP_FTP_CLIENT_

#include "tcpclient.h"

#include <list>
#include <string>
#include <utility>
#include <vector>

namespace miniftp {

const int DEFAULT_CTRL_PORT = 21;
const int MAX_TRY_TIMES = 5;

class FTPClient {
 public:
  FTPClient(const std::string& remote_addr, const std::string& remote_dir, int port);
  
 public:
  bool Connect();
  const std::string& Pwd();
  std::string Lsfile(const std::string& path);
  bool Chdir(const std::string& dir);
  bool Put(const std::string& local, const std::string& remote);
  bool Get(const std::string& remote, const std::string& local);
  void Quit();

 private:
  std::vector<std::string> split(const std::string& str, char separator);

 private:
  int ctrl_port_;
  int data_port_;
  std::string server_ip_;
  std::string current_path_;
  TCPClient ctrl_client_;
  TCPClient data_client_;
};

}

#endif
