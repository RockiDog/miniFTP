#include "ftpclient.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

//#define DEBUG_MODE 

using namespace miniftp;
using namespace std;

namespace {

FTPClient* g_ftpclient = 0;
int g_port = DEFAULT_CTRL_PORT;
string g_hostname;
string g_init_dir = "\\";
string g_current_path;

}

void PrintHelp() {
  printf("help\n");
}

vector<string> split(const string& str, char separator) {
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


int Main(int argc, char** argv) {

#ifndef DEBUG_MODE
  if (argc == 1) {
    PrintHelp();
    return 0;
  }
#endif

  /*****************************/
  /* Initialization Parameters */
  /*****************************/
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      PrintHelp();
      return 0;
    } else if (strcmp(argv[i], "-p") == 0) {
      g_port = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-d") == 0) {
      g_init_dir = argv[++i];
    } else {
      g_hostname = argv[i];
    }
  }

  /*************************/
  /* Launch the FTP client */
  /*************************/
#ifndef DEBUG_MODE
  g_ftpclient = new FTPClient(g_hostname, g_init_dir, g_port);
  printf("Connecting to the server %s\\%s:%d...\n", g_hostname.c_str(), g_init_dir.c_str(), g_port);
  if (g_ftpclient->Connect() == false) {
    printf("Cannot connect to the server!\n");
    return 0;
  } else {
    printf("Connecting to the server succeeded!\n");
  }
#endif

  while (true) {
    cout << ">>> ";
    string command;
    getline(cin, command);

    /*********************/
    /* Parse the command */
    /*********************/
    vector<string> parameters = split(command, ' ');
    assert(parameters.size() > 0);

    /***************************/
    /* QUIT to quit the client */
    /***************************/
    if (parameters[0] == "quit" || parameters[0] == "exit") {
      break;

    /*********************************/
    /* PWD to print the current path */
    /*********************************/
    } else if (parameters[0] == "pwd") {
      cout << g_ftpclient->Pwd() << endl;

    /*********************************************/
    /* LS to list the files under the given path */
    /*********************************************/
    } else if (parameters[0] == "ls") {
      if (parameters.size() > 1)
        cout << g_ftpclient->Lsfile(parameters[1]) << endl;
      else
        cout << g_ftpclient->Lsfile(".") << endl;

    /***********************/
    /* PUT to upload files */
    /***********************/
    } else if (parameters[0] == "put") {
      switch (parameters.size()) {
        case 1:
          cout << "Too few parameters!" << endl;
          PrintHelp();
          break;
        case 2:
          g_ftpclient->Put(parameters[1], ".\\");
          break;
        case 3:
          g_ftpclient->Put(parameters[1], parameters[2]);
          break;
        default:
          cout << "Too many parameters!" << endl;
          PrintHelp();
          break;
      }

    /*************************/
    /* GET to download files */
    /*************************/
    } else if (parameters[0] == "get") {
      switch (parameters.size()) {
        case 1:
          cout << "Too few parameters!" << endl;
          PrintHelp();
          break;
        case 2:
          g_ftpclient->Get(parameters[1], parameters[1]);
          break;
        case 3:
          g_ftpclient->Get(parameters[1], parameters[2]);
          break;
        default:
          cout << "Too many parameters!" << endl;
          PrintHelp();
          break;
      }

    /**************************/
    /* CD to change directory */
    /**************************/
    } else if (parameters[0] == "cd") {
      if (parameters.size() > 1)
        g_ftpclient->Chdir(parameters[1]);
      else
        g_ftpclient->Chdir("\\");

    /**************************************/
    /* HELP to print the help information */
    /**************************************/
    } else if (parameters[0] == "help") {
      PrintHelp();

    /************************************/
    /* WRONG to print error information */
    /************************************/
    } else {
      cout << "Wrong command!" << endl;
      PrintHelp();
    }
  }

#ifndef DEBUG_MODE
  delete g_ftpclient;
#endif

  return 0;
}
