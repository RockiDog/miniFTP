#include "tcpclient.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

using namespace miniftp;
using namespace std;

TCPClient::TCPClient() : is_connected_(false) {}

TCPClient::TCPClient(const string& ip_addr, const int& port)
    : host_name_(ip_addr), port_(port), is_connected_(false) {}

TCPClient::~TCPClient() {
  if (is_connected_ == true)
    Close();
}

bool TCPClient::Conn(const std::string& ip_addr, const int& port) {
  host_name_ = ip_addr;
  port_ = port;
  return Conn();
}

bool TCPClient::Conn() {

  /*****************************/
  /* Initialize Windows socket */
  /*****************************/
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0)
    return false;

  /***************************************/
  /* Resolve the server address and port */
  /***************************************/
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port_);
  if (InetPton(AF_INET, host_name_.c_str(), &server_addr_.sin_addr.s_addr) != 1) {
    printf("Cannot resolve the host name: %s:%d\n", host_name_.c_str(), port_);
    return false;
  }

  /*******************/
  /* Create a socket */
  /*******************/
  socket_client_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_client_ == INVALID_SOCKET) {
    printf("Invalid socket!\n");
    Close();
    return false;
  }

  /**********************/
  /* Attempt to connect */
  /**********************/
  if (connect(socket_client_, (const SOCKADDR*)(&server_addr_), sizeof(server_addr_)) == SOCKET_ERROR) {
    printf("Failed to connect!\n");
    Close();
    return false;
  }

  is_connected_ = true;
  return true;
}

void TCPClient::Close() {
  closesocket(socket_client_);
  WSACleanup();
  is_connected_ = false;
}

bool TCPClient::Send(const Byte* buffer, long long length) {
  const Byte* cursor = buffer;
  if (is_connected_ == false)
    return false;

  /***************************/
  /* Send the length of data */
  /***************************/
  if (send(socket_client_, (const Byte*)(&length), sizeof(length), 0) == SOCKET_ERROR) {
    Close();
    return false;
  }

  /********************/
  /* Send the content */
  /********************/
  for (int i = 0; i < length / MAX_INT; ++i) {
    if (send(socket_client_, cursor, MAX_INT, 0) == SOCKET_ERROR) {
      Close();
      return false;
    }
    cursor += MAX_INT;
  }
  if (length % MAX_INT != 0) {
    if (send(socket_client_, cursor, length % MAX_INT, 0) == SOCKET_ERROR) {
      Close();
      return false;
    }
  }

  return true;
}

bool TCPClient::Send(const list< pair<Byte*, int> >& blocks, long long length) {
  if (is_connected_ == false)
    return false;

  /***************************/
  /* Send the length of data */
  /***************************/
  if (send(socket_client_, (const Byte*)(&length), sizeof(length), 0) == SOCKET_ERROR) {
    Close();
    return false;
  }

  /********************/
  /* Send the content */
  /********************/
  for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it) {
    if (send(socket_client_, it->first, it->second, 0) == SOCKET_ERROR) {
      Close();
      return false;
    }
  }

  return true;
}

bool TCPClient::Recv(Byte* buffer, long long length) {
  Byte* cursor = buffer;
  int buffer_size;

  /****************************************/
  /* In case that the length is too large */
  /****************************************/
  for (int i = 0; i < length / MAX_INT; ++i) {
    buffer_size = MAX_INT;
    while (buffer_size > 0) {
      int result = recv(socket_client_, cursor, buffer_size, 0);
      if (result > 0) {
        buffer_size -= result;
        printf("%d bytes received...\n", result);
      } else if (result == 0) {
        printf("Connection closed!\n");
        return false;
      } else {
        printf("Receive failed!\n");
        return false;
      }
    }
  }
  buffer_size = length % MAX_INT;
  while (buffer_size > 0) {
    int result = recv(socket_client_, cursor, buffer_size, 0);
    if (result > 0) {
      buffer_size -= result;
      printf("%d bytes received...\n", result);
    } else if (result == 0) {
      printf("Connection closed!\n");
      return false;
    } else {
      printf("Receive failed!\n");
      return false;
    }
  }

  return true;
}

bool TCPClient::Recv(const list< pair<Byte*, int> >& blocks, long long length) {
  for (list< pair<Byte*, int> >::const_iterator it = blocks.cbegin(); it != blocks.cend(); ++it) {
    int buffer_size = it->second;
    while (buffer_size > 0) {
      int result = recv(socket_client_, it->first, buffer_size, 0);
      if (result > 0) {
        buffer_size -= result;
        printf("%d bytes received...\n", result);
      } else if (result == 0) {
        printf("Connection closed!\n");
        return false;
      } else {
        printf("Receive failed!\n");
        return false;
      }
    }
  }

  return true;
}
