#include "ftpclient.h"
#include "tcpclient.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <utility>
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
    if (tried_times == 0)
      printf("Trying the first time...\n");
    else
      printf("Trying %d times...\n", tried_times + 1);

    if (ctrl_client_.Conn() == true)
      return true;
    ++tried_times;
    Sleep(3000);
  }
  return false;
}

const string& FTPClient::Pwd() {
  return current_path_;
}

string FTPClient::Lsfile(const string& path) {

  /*************************/
  /* Send the control word */
  /*************************/
  string ctrl_wd = "LIST:" + path;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /*********************/
    /* Get the file list */
    /*********************/
    long long length;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      Byte* buffer = new Byte[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /*************************************************************/
        /* The string begins with a '?' represents an error occurred */
        /*************************************************************/
        if (buffer[0] == '?') {
          string error_message(buffer + 1);
          delete [] buffer;
          return error_message;
        } else {
          string filelist(buffer);
          delete [] buffer;
          return filelist;
        }
      }
      delete [] buffer;
    } else {
      printf("Failed to Receive!\n");
    }
  } else {
    printf("Connection lost!\n");
  }
  return string("Relaunch and try again!\n");
}

bool FTPClient::Chdir(const std::string& dir) {

  /*************************/
  /* Send the control word */
  /*************************/
  string ctrl_wd = "CD:" + dir;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /**********************************/
    /* Get the acknowledgment message */
    /**********************************/
    long long length;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      char* buffer = new char[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /*************************************************************/
        /* The string begins with a '?' represents an error occurred */
        /*************************************************************/
        if (buffer[0] == '?') {
          printf("%s\n", buffer + 1);
        } else {
          current_path_ = buffer;
          delete [] buffer;
          return true;
        }
      }
      delete [] buffer;
    }
  }
  return false;
}

bool FTPClient::Put(const std::string& local, const std::string& remote) {

  /***********************************/
  /* Make sure the local file exists */
  /***********************************/
  ifstream ifs(local, ifstream::ate | ifstream::binary);
  if (!ifs) {
    printf("File %s not exists!\n");
    return false;
  }
  ifs.seekg(0);

  /*****************/
  /* Read the file */
  /*****************/
  streamsize filesize = ifs.tellg();
  list< pair<Byte*, int> > blocks;
  for (int i = 0; i < filesize / MAX_INT; ++i) {
    Byte* block = new Byte[MAX_INT];
    ifs.read(block, MAX_INT);
    blocks.push_back(make_pair(block, MAX_INT));
  }
  if (filesize % MAX_INT != 0) {
    Byte* block = new Byte[filesize % MAX_INT];
    ifs.read(block, filesize % MAX_INT);
    blocks.push_back(make_pair(block, filesize % MAX_INT));
  }

  /*************************/
  /* Send the control word */
  /*************************/
  string ctrl_wd = "PUT:" + remote;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /**********************************/
    /* Get the acknowledgment message */
    /**********************************/
    long long length;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      Byte* buffer = new char[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /*************************************************************/
        /* The string begins with a '?' represents an error occurred */
        /*************************************************************/
        if (buffer[0] == '?') {
          printf("%s\n", buffer + 1);
        } else {
          data_port_ = atoi(buffer);

          /**********************************************/
          /* Return false after tried for several times */
          /**********************************************/
          int tried_times = 0;
          while (tried_times < MAX_TRY_TIMES) {
            if (data_client_.Conn(server_ip_, data_port_) == true) {

              /*********************/
              /* Transfer the file */
              /*********************/
              if (data_client_.Send(blocks, filesize) == true) {

                /************************************/
                /* Wait for the acknowledge message */
                /************************************/
                long long byte_count;
                if (ctrl_client_.Recv((Byte*)(&byte_count), sizeof(byte_count)) == true) {
                  if (byte_count == filesize) {
                    for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it)
                      delete [] it->first;
                    delete [] buffer;
                    ifs.close();
                    data_client_.Close();
                    return true;
                  } else {
                    printf("Wrong bytes number!\n");
                    break;
                  }
                } else {
                  printf("Connection lost!\n");
                  break;
                }
              } else {
                printf("Failed to send!\n");
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
  for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it)
    delete[] it->first;
  ifs.close();
  data_client_.Close();
  return false;
}

bool FTPClient::Get(const std::string& remote, const std::string& local) {

  /*******************************************/
  /* Make sure the local file can be created */
  /*******************************************/
  ofstream ofs(local);
  if (!ofs) {
    printf("Can not create local file!\n");
    return false;
  }

  /*************************/
  /* Send the control word */
  /*************************/
  string ctrl_wd = "GET:" + remote;
  if (ctrl_client_.Send(ctrl_wd.c_str(), ctrl_wd.length()) == true) {

    /**********************************/
    /* Get the acknowledgment message */
    /**********************************/
    long long length;
    if (ctrl_client_.Recv((Byte*)(&length), sizeof(length)) == true) {
      Byte* buffer = new char[length + 1];
      buffer[length] = '\0';
      if (ctrl_client_.Recv(buffer, length) == true) {

        /*************************************************************/
        /* The string begins with a '?' represents an error occurred */
        /*************************************************************/
        if (buffer[0] == '?') {
          printf("%s\n", buffer + 1);
        } else {
          data_port_ = atoi(buffer);

          /**********************************************/
          /* Return false after tried for several times */
          /**********************************************/
          int tried_times = 0;
          while (tried_times < MAX_TRY_TIMES) {
            if (data_client_.Conn(server_ip_, data_port_) == true) {

              /*********************/
              /* Transfer the file */
              /*********************/
              streamsize filesize;
              if (data_client_.Recv((Byte*)(&filesize), sizeof(filesize)) == true) {

                /*********************/
                /* Create block list */
                /*********************/
                list< pair<Byte*, int> > blocks;
                for (int i = 0; i < filesize / MAX_INT; ++i) {
                  Byte* block = new Byte[MAX_INT];
                  blocks.push_back(make_pair(block, MAX_INT));
                }
                if (filesize % MAX_INT != 0) {
                  Byte* block = new Byte[filesize % MAX_INT];
                  blocks.push_back(make_pair(block, filesize % MAX_INT));
                }

                /********************/
                /* Receive the file */
                /********************/
                if (data_client_.Recv(blocks, filesize) == true) {

                  /***************************************/
                  /* Wait for the acknowledgment message */
                  /***************************************/
                  long long byte_count;
                  if (ctrl_client_.Recv((Byte*)(&byte_count), sizeof(byte_count)) == true) {
                    if (byte_count == filesize) {

                      /**************************/
                      /* Write back to the disk */
                      /**************************/
                      for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it) {
                        ofs.write(it->first, it->second);
                        delete [] it->first;
                      }
                      delete [] buffer;
                      ofs.close();
                      data_client_.Close();
                      return true;
                    } else {
                      for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it)
                        delete [] it->first;
                      printf("Wrong bytes number!\n");
                      break;
                    }
                  } else {
                    for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it)
                      delete [] it->first;
                    printf("Connection lost!\n");
                    break;
                  }
                } else {
                  for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it)
                    delete [] it->first;
                  printf("Failed to Receive!\n");
                  break;
                }
              } else {
                printf("Failed to Receive!\n");
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
  ofs.close();
  data_client_.Close();
  return false;
}

void FTPClient::Quit() {
  ctrl_client_.Close();
  data_client_.Close();
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