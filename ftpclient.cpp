#include "ftpclient.h"
#include "tcpclient.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace miniftp;
using namespace std;

FTPClient::FTPClient(const string& remote_addr, const string& remote_dir, int port)
    : ctrl_port_(port),
      data_port_(port - 1),
      server_ip_(remote_addr),
      current_path_(remote_dir),
      ctrl_client_(remote_addr, port) {}

bool FTPClient::Connect() {
  int tried_times = 0;
  while (tried_times < MAX_TRY_TIMES) {
    if (ctrl_client_.Conn() == true) {
      if (SetCurrentDirectory(current_path_.c_str()) == false) {
        printf("GetCurrentDirectory failed(%d)\n", GetLastError());
        break;
      }
      return true;
    }
    ++tried_times;
    Sleep(3000);
  }
  return false;
}

string FTPClient::Pwd() {
  return current_path_;
}

string FTPClient::List() {

}

bool FTPClient::Chdir(const std::string& dir) {

  /* Send the control word */
  string ctrl_wd = "CD:" + dir;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /* Get the acknowledgment message */
    long long length = 0;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      char* buffer = new char[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /* The string begins with a '?' represents an error occurred */
        if (buffer[0] == '?') {
          printf("%s\n", buffer + 1);
        } else {
          current_path_ = buffer;
          if (SetCurrentDirectory(current_path_.c_str()) == true) {
            delete [] buffer;
            return true;
          } else {
            printf("GetCurrentDirectory failed(%d)\n", GetLastError());
          }
        }
      }
      delete [] buffer;
    }
  }
  return false;
}

bool FTPClient::Put(const std::string& local, const std::string& remote) {

  /* Make sure the local file exists */
  ifstream ifs(local, ifstream::ate | ifstream::binary);
  if (!ifs == true) {
    printf("File %s not exists!\n");
    return false;
  }

  /* Get the file size */
  streamsize filesize = ifs.tellg();
  Byte* bytes = new Byte[filesize];
  ifs.seekg(0);
  ifs.read(bytes, filesize);

  /* Send the control word */
  string ctrl_wd = "PUT:" + remote;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /* Get the acknowledgment message */
    long long length = 0;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      char* buffer = new char[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /* The string begins with a '?' represents an error occurred */
        if (buffer[0] == '?') {
          printf("%s\n", buffer + 1);
        } else {
          data_port_ = toint(buffer);

          /* Return false after tried for some times */
          int tried_times = 0;
          while (tried_times < MAX_TRY_TIMES) {
            if (data_client_.Conn(server_ip_, data_port_) == true) {
              if (data_client_.Send(bytes, filesize) == false)
                break;

              /* Wait for the acknowledge message */
              long long bytes_sent;
              if (ctrl_client_.Recv((Byte*)bytes_sent, sizeof(bytes_sent)) == true) {
                if (bytes_sent == filesize) {
                  delete [] buffer;
                  return true;
                } else {
                  printf("Wrong bytes number!\n");
                  break;
                }
              } else {
                printf("Connection lost!\n");
                break;
              }
            }
            ++tried_times;
            Sleep(3000);
          }
        }
      }
      delete [] buffer;
    }
  }
  return false;
}

bool FTPClient::Get(const std::string& remote, const std::string& local) {
}

vector<string> FTPClient::split(const string& str, char separator) {
  string s = str;
  if (s.back() == separator)
    s.pop_back();
  char* sub_string = new char[str.length() + 1];
  int sub_len = 0;
  vector<string> strings;

  for (string::const_iterator& it = str.cbegin(); it != str.cend(); ++it) {
    if (*it == separator) {
      sub_string[sub_len] = '\0';
      strings.push_back(sub_string);
      sub_len = 0;
    } else if (it == str.cend() - 1) {
      sub_string[sub_len++] = *it;
      sub_string[sub_len] = '\0';
      strings.push_back(sub_string);
    } else {
      sub_string[sub_len++] = *it;
    }
  }

  delete [] sub_string;
  return strings;
}

int FTPClient::toint(const std::string& str) {
  int num = 0;
  for (string::const_iterator it = str.cbegin(); it != str.cend(); ++it) {
    num *= 10;
    num += *it - '0';
  }
  return num;
}
