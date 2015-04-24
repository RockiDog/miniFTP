#include "ftpserver.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace miniftp;
using namespace std;

/*
FTPServer::FTPServer() {
  int len = GetCurrentDirectory(0, 0);
  char* dir = new char[len];
  if (GetCurrentDirectory(len, dir) != 0) {
    root_path_ = dir;
    current_path_ = dir;
  } else {
    printf("GetCurrentDirectory failed(%d)\n", GetLastError());
  }
  delete [] dir;
}

const string& FTPServer::pwd() {
  return current_path_;
}


bool FTPServer::chdir(const string& dir) {
  const string* d = 0;
  if (dir.length() == 0)
    d = &root_path_;
  else
    d = &dir;

  if (SetCurrentDirectory(d->c_str()) == false) {
    printf("GetCurrentDirectory failed(%d)\n", GetLastError());
    return false;
  }
  current_path_ = *d;
  return true;
}

bool FTPServer::list(const string& dir) {

}
*/

/*
vector<string> FTPServer::split(const string& str, char separator) {
  string s = str;
  if (s.back() == separator)
    s.pop_back();
  char* sub_string = new char[str.length()];
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
*/