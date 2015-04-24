#ifndef MINIFTP_FTP_SERVER_
#define MINIFTP_FTP_SERVER_

#include <string>

namespace miniftp {

class FTPServer {
 public:
  FTPServer();
  ~FTPServer();

 private:
  const std::string& pwd();
  bool put(const std::string& local, const std::string& remote);
  bool get(const std::string& remote, const std::string& local);
  bool chdir(const std::string& dir = "");
  bool list(const std::string& dir = "");
  bool quit();

  //std::vector<std::string> split(const std::string& str, char separator);
  //void updatedir();

 private:
  std::string current_path_;
  std::string root_path_;
};

}

#endif
