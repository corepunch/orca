#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "someip.h"
#include "vsomeip.h"

#include <include/orcadef.h>
#include <libxml/tree.h>

#define PK_READ(SB, VAR) PK_Read(SB, &VAR, sizeof(VAR))

typedef struct
{
  void* Buffer;
  uint32_t Size;
  uint32_t Position;
} PACKET, *LPPACKET;

int net_socket_has_data(int sockfd)
{
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(sockfd, &read_fds);
  struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
  int i = select(sockfd + 1, &read_fds, NULL, NULL, &tv);
  if (i == -1) {
    perror("select");
    return -1;
  }
  if (i > 0) {
    return 1;
  }
  return i;
}

bool_t
PK_Write(LPPACKET hPak, void* lpValue, uint32_t dwSize)
{
  if (hPak->Position + dwSize > hPak->Size) {
    return FALSE;
  }
  memcpy(hPak->Buffer + hPak->Position, lpValue, dwSize);
  hPak->Position += dwSize;
  return TRUE;
}

bool_t
PK_Read(LPPACKET hPak, void* lpValue, uint32_t dwSize)
{
  if (hPak->Position + dwSize > hPak->Size) {
    return FALSE;
  }
  memcpy(lpValue, hPak->Buffer + hPak->Position, dwSize);
  hPak->Position += dwSize;
  return TRUE;
}

LPPACKET
PK_Extract(LPPACKET hPak, LPPACKET hOther, uint32_t dwSize)
{
  if (hPak->Position + dwSize > hPak->Size) {
    return NULL;
  }
  hOther->Buffer = hPak->Buffer + hPak->Position;
  hOther->Size = dwSize;
  hOther->Position = 0;
  hPak->Position += dwSize;
  return hOther;
}

void
handle_error(char const* message)
{
  perror(message);
  exit(EXIT_FAILURE);
}

int open_vsomeipd(int app_id)
{
  // Create a Unix socket for vsomeip-0
  int sockfd_0 = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd_0 == -1) {
    perror("socket");
    return -1;
  }

  // Set up the server address structure for vsomeip-0
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/vsomeip-%x", app_id);

  // Connect to vsomeip-0
  if (connect(sockfd_0, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    //        perror("connect");
    close(sockfd_0);
    return -1;
  }

  Con_Error("Connected to /tmp/vsomeip-%x.", app_id);

  return sockfd_0;
}

int send_packet(int sockfd, packet_header_t* header, void* payload)
{
  uint8_t buffer[256];
  PACKET hPak = { .Buffer = buffer, .Size = sizeof(buffer) };
  uint32_t _header = VSOMEIP_PACKET_BEGIN;
  uint32_t _footer = VSOMEIP_PACKET_END;

  PK_Write(&hPak, &_header, sizeof(_header));
  PK_Write(&hPak, header, sizeof(*header));
  PK_Write(&hPak, payload, header->payload_size);
  PK_Write(&hPak, &_footer, sizeof(_footer));

  // Send data to vsomeip-0
  ssize_t bytesSent = write(sockfd, hPak.Buffer, hPak.Position);
  if (bytesSent == -1) {
    static int logged = FALSE;
    if (!logged) {
      perror("write");
      logged = TRUE;
    }
    return FALSE;
  }

  //    Con_Error("Data sent to %s.", SOCKET_PATH_0);

  return TRUE;
}

void
read_RIE_ADD_CLIENT(LPMYSERVICE hService, LPPACKET hPak, client_t client)
{
  Con_Error("Added client %d", client);

  send_packet(hService->fout,
              &(packet_header_t){
                .client_id = hService->ident,
                .cmd_id = VSOMEIP_REGISTERED_ACK,
                .payload_size = 0,
              },
              NULL);

  hService->on_client_added(hService, client);
}

void
VP_ReadService(LPMYSERVICE hService, LPPACKET hPak, client_t client)
{
  service_data_t service = { .use_exclusive_proxy_ = TRUE };
  if (!PK_READ(hPak, service.service_))
    return;
  while (PK_READ(hPak, service.instance_) && PK_READ(hPak, service.major_) &&
         PK_READ(hPak, service.minor_)) {
    hService->on_service_added(hService, &service);
  }
}

void
read_RIE_ADD_SERVICE_INSTANCE(LPMYSERVICE hService,
                              LPPACKET hPak,
                              client_t client)
{
  uint32_t size;
  PACKET block;
  if (!PK_READ(hPak, size))
    return;
  return VP_ReadService(hService, PK_Extract(hPak, &block, size), client);
}

void
read_ROUTING_INFO(LPMYSERVICE hService, LPPACKET hPak)
{
  routing_info_entry_e routing_info_entry;
  uint32_t size;
  client_t client;
  PACKET block;
  if (!PK_READ(hPak, routing_info_entry))
    return;
  if (!PK_READ(hPak, size))
    return;
  if (!PK_Extract(hPak, &block, size))
    return;
  if (PK_READ(&block, client)) {
    switch (routing_info_entry) {
      case RIE_ADD_CLIENT:
        read_RIE_ADD_CLIENT(hService, &block, client);
        break;
      case RIE_ADD_SERVICE_INSTANCE:
        read_RIE_ADD_SERVICE_INSTANCE(hService, &block, client);
        break;
      default:
        Con_Error("Unknown VSOMEIP_ROUTING_INFO %02x", routing_info_entry);
        break;
    }
  }
}

void
read_SUBSCRIBE_ACK(LPMYSERVICE hService, LPPACKET hPak)
{
  subscribe_ack_t ack;
  PK_READ(hPak, ack);
  Con_Error("SUBSCIBE_ACK(%04x): [%04x.%04x.%04x.%04x]",
         ack.subscriber,
         ack.service,
         ack.instance,
         ack.eventgroup,
         ack.event);
}

extern xmlDocPtr cardata;

xmlChar const*
xmlGetPropQuick(xmlNodePtr node, xmlChar const* name);

static xmlNodePtr
_FindService(long dwService)
{
  if (!cardata)
    return NULL;
  xmlForEach(service, xmlDocGetRootElement(cardata))
  {
    if (_CHECKTYPE(service, type_service) &&
        dwService == LOWORD(_GETVALUE(service)))
      return service;
  }
  return NULL;
}

static xmlNodePtr
_FindMethod(xmlNodePtr service, long dwMethod)
{
  if (!service)
    return NULL;
  xmlForEach(method, service)
  {
    if (dwMethod == LOWORD(_GETVALUE(method))) {
      return method;
    }
  }
  return NULL;
}

typedef struct _BUFFER
{
  long length;
  void* data;
}* PBUFFER;

#define READ_MEM(SIZE)                                                         \
  if (buffer->length >= SIZE) {                                                \
    memcpy(&node->_private, buffer->data, SIZE);                               \
    buffer->length -= SIZE;                                                    \
    buffer->data += SIZE;                                                      \
  }

#define READ_ATOM(TYPE)                                                        \
  else if (!xmlStrcmp(node->name, BAD_CAST #TYPE))                             \
  {                                                                            \
    READ_MEM(sizeof(TYPE##_t));                                                \
  }

static long
_ReadNode(xmlNodePtr node, PBUFFER buffer)
{
  if (node->children) {
    xmlForEach(child, node)
    {
      _ReadNode(child, buffer);
    }
  }
  READ_ATOM(bool)
  READ_ATOM(int8)
  READ_ATOM(uint8)
  READ_ATOM(int16)
  READ_ATOM(uint16)
  READ_ATOM(int32)
  READ_ATOM(uint32)
  READ_ATOM(int64)
  READ_ATOM(uint64)
  else if (node->extra == 1)
  {
    READ_MEM(sizeof(int));
  }
  else
  {
    Con_Error("Ignoring %s", (char const*)node->name);
  }
  return buffer->length;
}

int Read_Service(long dwService, long dwMethod, long dwLength, void* lpData)
{
  xmlWith(xmlNode, method, _FindMethod(_FindService(dwService), dwMethod), _keep)
  {
    if (_ReadNode(method, &(struct _BUFFER){ dwLength, lpData })) {
      Con_Error("Error reading %s packet of %d bytes\n",
              (char const*)method->name,
              (int)dwLength);
    }
  }
  return FALSE;
}

void
on_client_added(LPMYSERVICE hService, client_t client)
{
  if (!cardata)
    return;
  xmlForEach(service, xmlDocGetRootElement(cardata))
  {
    if (_CHECKTYPE(service, type_service)) {
      send_packet(hService->fout,
                  &(packet_header_t){ .client_id = hService->ident,
                                      .cmd_id = VSOMEIP_REQUEST_SERVICE,
                                      .payload_size = sizeof(service_data_t) },
                  &(service_data_t){
                    .service_ = LOWORD(_GETVALUE(service)),
                    .instance_ = 1,
                    .minor_ = 1,
                    .major_ = -1,
                  });
    }
  }
}

void
on_service_added(LPMYSERVICE hService, service_data_t* sv)
{
  xmlForEach(service, xmlDocGetRootElement(cardata))
  {
    if (_CHECKTYPE(service, type_service) &&
        sv->service_ == LOWORD(_GETVALUE(service))) {
      xmlForEach(method, service)
      {
        send_packet(hService->fout,
                    &(packet_header_t){
                      .client_id = hService->ident,
                      .cmd_id = VSOMEIP_SUBSCRIBE,
                      .payload_size = sizeof(subscribe_cmd_t),
                    },
                    &(subscribe_cmd_t){
                      .service = sv->service_,
                      .instance = sv->instance_,
                      .eventgroup = HIWORD(_GETVALUE(method)),
                      .event = LOWORD(_GETVALUE(method)),
                      .major = sv->major_,
                      .subscription_type = SU_RELIABLE_AND_UNRELIABLE,
                      .pending_subscription_id = 0,
                    });
      }
    }
  }
}

void
read_SEND(LPMYSERVICE hService, LPPACKET hPak)
{
  send_cmd_t send_cmd;
  command_header_t command_header;

  PK_READ(hPak, send_cmd);
  PK_READ(hPak, command_header);

  // Con_Error("service %x", htons(command_header.service));
  // Con_Error("method %x", htons(command_header.method));
  // Con_Error("length %x", htonl(command_header.length));
  // Con_Error("message_type %x", command_header.message_type);

  switch (command_header.message_type) {
    case MT_NOTIFICATION:
      Read_Service(htons(command_header.service),
                   htons(command_header.method),
                   htonl(command_header.length) - 8,
                   hPak->Buffer + hPak->Position);
      break;
  }
}

bool_t
PK_ReadTag(LPPACKET hPak, uint32_t tag)
{
  uint32_t value = 0;
  PK_READ(hPak, value);
  return tag == value;
}

bool_t
read_vsomeip_packet(LPMYSERVICE hService, int sockfd)
{
  static unsigned char received_data[VSOMEIP_BUFFER_SIZE];

  uint32_t tag;
  packet_header_t header;
  //	PACKET payload;

  if (read(sockfd, &tag, sizeof(tag)) <= 0 || tag != VSOMEIP_PACKET_BEGIN) {
    puts("Incorrect opening tag");
    return FALSE;
  }

  if (read(sockfd, &header, sizeof(header)) <= 0) {
    return FALSE;
  }

  assert(header.payload_size <= VSOMEIP_BUFFER_SIZE);

  if (read(sockfd, &received_data, header.payload_size) <= 0) {
    return FALSE;
  }

  if (read(sockfd, &tag, sizeof(tag)) <= 0 || tag != VSOMEIP_PACKET_END) {
    puts("Incorrect closing tag");
    return FALSE;
  }

  PACKET p = { .Buffer = received_data,
               .Size = header.payload_size,
               .Position = 0 };

  // Con_Error("%x %x %x", header.cmd_id, header.client_id,
  // header.payload_size);

  switch (header.cmd_id) {
    case VSOMEIP_ROUTING_INFO:
      read_ROUTING_INFO(hService, &p);
      break;
    case VSOMEIP_SUBSCRIBE_ACK:
      read_SUBSCRIBE_ACK(hService, &p);
      break;
    case VSOMEIP_SEND:
      read_SEND(hService, &p);
      break;
    default:
      Con_Error("Unknown command %02x", header.cmd_id);
      return FALSE;
  }

  return TRUE;
}

void
RegisterApplication(LPMYSERVICE hService)
{
  if (hService->fout == -1) {
    hService->fout = open_vsomeipd(0);
  }
  if (hService->fout != -1 &&
      send_packet(hService->fout,
                  &(packet_header_t){
                    .cmd_id = VSOMEIP_REGISTER_APPLICATION,
                    .client_id = hService->ident,
                  },
                  NULL)) {
    hService->registered = TRUE;
  }
}

void
AcceptConnections(LPMYSERVICE hService)
{
  while (net_socket_has_data(hService->fin)) {
    int sockfd = accept(hService->fin, NULL, NULL);
    if (sockfd == -1) {
      handle_error("accept");
      return;
    }
    assert(hService->numclients < MAX_SERVICE_CLIENTS);
    hService->clients[hService->numclients++] = sockfd;
    puts("Connection accepted.");
  }
}

void
ReadClients(LPMYSERVICE hService)
{
  for (int i = 0; i < hService->numclients; i++) {
    int sockfd = hService->clients[i];
    while (net_socket_has_data(sockfd)) {
      // Receive data from vsomeip-3
      if (!read_vsomeip_packet(hService, sockfd)) {
        close(sockfd);
        for (int j = i; j < hService->numclients - 1; j++) {
          hService->clients[j] = hService->clients[j + 1];
        }
        hService->numclients--;
        puts("Dropped client");
        break;
      }
    }
  }
}

int OpenListeningSocket(char const* filename)
{
  // Create a Unix socket for vsomeip-3
  int sockfd_3 = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd_3 == -1) {
    handle_error("socket");
  }

  Con_Error("Socket created.");

  // Set up the server address structure for vsomeip-3
  struct sockaddr_un server_addr_3;
  server_addr_3.sun_family = AF_UNIX;
  strncpy(server_addr_3.sun_path, filename, sizeof(server_addr_3.sun_path) - 1);

  unlink(filename);

  if (bind(sockfd_3, (struct sockaddr*)&server_addr_3, sizeof(server_addr_3)) ==
      -1) {
    handle_error("bind");
  }

  Con_Error("Socket bound to %s.", filename);

  // Listen for incoming connections
  if (listen(sockfd_3, 1) == -1) {
    handle_error("listen");
  }

#ifdef __QNX__
  // Set socket file permissions
  if (chmod(filename, 0666) == -1) {
    handle_error("chmod socket file");
  }
#endif

  Con_Error("Server is listening on %s...", filename);

  return sockfd_3;
  ;
}

void
CloseListeningSocket(int sockfd)
{
  struct sockaddr_un addr;
  socklen_t addr_len = sizeof(addr);
  if (getsockname(sockfd, (struct sockaddr*)&addr, &addr_len) == -1) {
    perror("getsockname");
    return;
  }
  close(sockfd);
  unlink(addr.sun_path);
}
