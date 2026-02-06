#ifndef someip_h
#define someip_h

#include "vsomeip.h"

#if defined(__APPLE__)
#define SD_DEVICE "en1"
#elif defined(__QNX__)
#define SD_DEVICE "vlan210"
#endif

#ifdef __QNX__
#define SD_SUBSCRIBE_IP "10.210.220.2"
#else
#define SD_SUBSCRIBE_IP "192.168.2.118"
#endif

#define MAX_SERVICE_CLIENTS 256

typedef struct _MYSERVICE* LPMYSERVICE;

struct _MYSERVICE
{
  int ident;
  int fout;
  int fin;
  int port;
  int numclients;
  int registered;
  char ipaddress[4];
  void (*on_client_added)(LPMYSERVICE, client_t client);
  void (*on_service_added)(LPMYSERVICE, service_data_t*);
  int clients[MAX_SERVICE_CLIENTS];
};

int read_service(struct _MYSERVICE* sv, char* buffer, long size);
int set_ip(struct _MYSERVICE* sv);
int get_socket_port(int sockfd);
int SD_Listen(void);
void
SD_Subscribe(void* data, long size);
void
get_local_ip(char const* ifname, char* ip);
void
shutdown_service(struct _MYSERVICE* sv);

#endif
