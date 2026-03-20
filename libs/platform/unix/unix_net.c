#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include "../platform.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SA struct sockaddr

int net_open_socket(int port)
{
  int sockfd;
  struct sockaddr_in servaddr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 0;
  }

  int one = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  memset(&servaddr, 0, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
    perror("bind");
    close(sockfd);
    return 0;
  }
  // Now server is ready to listen and verification
  if ((listen(sockfd, 5)) != 0) {
    perror("listen");
    close(sockfd);
    return 0;
  }
  return sockfd;
}

void
net_close_socket(int sockfd)
{
  close(sockfd);
}

int net_accept(int sockfd)
{
  return accept(sockfd, NULL, NULL);
}

void
NET_Sent(int sockfd, void const* buf, size_t len)
{
  send(sockfd, buf, len, 0);
}

int net_connect(char const *ipaddr, int port)
{
  int sockfd;
  struct sockaddr_in addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ipaddr);
  addr.sin_port = htons(port);

  //    if (net_set_nonblocking(sockfd) == -1)
  //    {
  //        close(sockfd);
  //        return -1;
  //    }

  // connect the client socket to server socket
  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect");
    close(sockfd);
    return -1;
  } else {
    fprintf(stderr, "Connected to a server at %s:%d\n", ipaddr, port);
  }
  return sockfd;
}

int net_packet(int net_socket, struct WI_Buffer* net_message)
{
  int ret = (int)read(net_socket, net_message->data, net_message->maxsize);
  if (ret == net_message->maxsize) {
    net_message->cursize = 0;
    fprintf(stderr, "Oversized packet from %d\n", net_socket);
    return -1;
  }
  if (ret > 0) {
    net_message->cursize = ret;
  } else {
    net_message->cursize = 0;
  }
  return ret;
}

int net_send_packet(int sock, struct WI_Buffer* net_message)
{
  return (int)write(sock, net_message->data, net_message->cursize);
}

// Function to set a socket to non-blocking mode
int net_set_nonblocking(int sockfd)
{
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl");
    return -1;
  }

  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl");
    return -1;
  }

  return 0;
}

bool_t
net_has_no_error(void)
{
  return errno == EWOULDBLOCK || errno == EAGAIN;
}
