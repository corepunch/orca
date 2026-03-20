/* Winsock2 must be included before windows.h to avoid conflicts */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../platform.h"

/* One-time Winsock initialisation */
static void
ensure_winsock(void)
{
  static int initialised = 0;
  if (!initialised) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
      fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
      return;
    }
    initialised = 1;
  }
}

int
net_open_socket(int port)
{
  ensure_winsock();

  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == INVALID_SOCKET) {
    fprintf(stderr, "socket: %d\n", WSAGetLastError());
    return 0;
  }

  int one = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
             (char const *)&one, sizeof(one));

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons((u_short)port);

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
    fprintf(stderr, "bind: %d\n", WSAGetLastError());
    closesocket(sockfd);
    return 0;
  }
  if (listen(sockfd, 5) == SOCKET_ERROR) {
    fprintf(stderr, "listen: %d\n", WSAGetLastError());
    closesocket(sockfd);
    return 0;
  }
  return (int)sockfd;
}

void
net_close_socket(int sockfd)
{
  closesocket((SOCKET)sockfd);
}

int
net_accept(int sockfd)
{
  return (int)accept((SOCKET)sockfd, NULL, NULL);
}

void
NET_Sent(int sockfd, void const *buf, size_t len)
{
  send((SOCKET)sockfd, (char const *)buf, (int)len, 0);
}

int
net_connect(char const *ipaddr, int port)
{
  ensure_winsock();

  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == INVALID_SOCKET) {
    fprintf(stderr, "socket: %d\n", WSAGetLastError());
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ipaddr);
  addr.sin_port        = htons((u_short)port);

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    fprintf(stderr, "connect: %d\n", WSAGetLastError());
    closesocket(sockfd);
    return -1;
  }
  fprintf(stderr, "Connected to %s:%d\n", ipaddr, port);
  return (int)sockfd;
}

int
net_packet(int net_socket, struct WI_Buffer *net_message)
{
  int ret = recv((SOCKET)net_socket,
                 (char *)net_message->data, net_message->maxsize, 0);
  if (ret == net_message->maxsize) {
    net_message->cursize = 0;
    fprintf(stderr, "Oversized packet from %d\n", net_socket);
    return -1;
  }
  net_message->cursize = (ret > 0) ? ret : 0;
  return ret;
}

int
net_send_packet(int sock, struct WI_Buffer *net_message)
{
  return send((SOCKET)sock,
              (char const *)net_message->data, net_message->cursize, 0);
}

int
net_set_nonblocking(int sockfd)
{
  u_long mode = 1;
  if (ioctlsocket((SOCKET)sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
    fprintf(stderr, "ioctlsocket: %d\n", WSAGetLastError());
    return -1;
  }
  return 0;
}

bool_t
net_has_no_error(void)
{
  int err = WSAGetLastError();
  return (err == WSAEWOULDBLOCK) ? TRUE : FALSE;
}
