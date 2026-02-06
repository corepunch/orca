#ifndef vsomeip_h
#define vsomeip_h

#include <stdint.h>
#include <stdio.h>

typedef uint8_t bool_t;

typedef uint32_t message_t;
typedef uint16_t service_t;
typedef uint16_t method_t;
typedef uint16_t event_t;

typedef uint16_t instance_t;
typedef uint16_t eventgroup_t;

typedef uint8_t major_version_t;
typedef uint32_t minor_version_t;

typedef uint32_t ttl_t;

typedef uint32_t request_t;
typedef uint16_t client_t;
typedef uint16_t session_t;

typedef uint32_t length_t;

typedef uint8_t protocol_version_t;
typedef uint8_t interface_version_t;

typedef uint8_t byte_t;
typedef uint8_t subscription_type_t;

typedef uint16_t pending_subscription_id_t;
typedef uint32_t pending_remote_offer_id_t;
typedef uint32_t pending_security_update_id_t;

#define FALSE 0
#define TRUE 1

#define VSOMEIP_BUFFER_SIZE 1024

#define VSOMEIP_PACKET_BEGIN 0x076D3767
#define VSOMEIP_PACKET_END 0x67376D07

#define VSOMEIP_ROUTING "@VSOMEIP_ROUTING@"
#define VSOMEIP_ROUTING_CLIENT 0
#define VSOMEIP_ROUTING_INFO_SIZE_INIT 256

#ifdef _WIN32
#define VSOMEIP_INTERNAL_BASE_PORT 51234
#define __func__ __FUNCTION__
#endif

#define VSOMEIP_UNICAST_ADDRESS "@VSOMEIP_UNICAST_ADDRESS@"
#define VSOMEIP_NETMASK "255.255.255.0"

#define VSOMEIP_DEFAULT_CONNECT_TIMEOUT 100
#define VSOMEIP_MAX_CONNECT_TIMEOUT 1600
#define VSOMEIP_DEFAULT_FLUSH_TIMEOUT 1000

#define VSOMEIP_DEFAULT_SHUTDOWN_TIMEOUT 5000

#define VSOMEIP_MAX_TCP_CONNECT_TIME 5000
#define VSOMEIP_MAX_TCP_RESTART_ABORTS 5

#define VSOMEIP_DEFAULT_BUFFER_SHRINK_THRESHOLD 5

#define VSOMEIP_DEFAULT_WATCHDOG_TIMEOUT 5000
#define VSOMEIP_DEFAULT_MAX_MISSING_PONGS 3

#define VSOMEIP_DEFAULT_UDP_RCV_BUFFER_SIZE 1703936

#define VSOMEIP_IO_THREAD_COUNT 2
#define VSOMEIP_IO_THREAD_NICE_LEVEL 255

#define VSOMEIP_MAX_DISPATCHERS 10
#define VSOMEIP_MAX_DISPATCH_TIME 100

#define VSOMEIP_MAX_DESERIALIZER 5

#define VSOMEIP_REQUEST_DEBOUNCE_TIME 10

#define VSOMEIP_COMMAND_HEADER_SIZE 7

#define VSOMEIP_COMMAND_TYPE_POS 0
#define VSOMEIP_COMMAND_CLIENT_POS 1
#define VSOMEIP_COMMAND_SIZE_POS_MIN 3
#define VSOMEIP_COMMAND_SIZE_POS_MAX 6
#define VSOMEIP_COMMAND_PAYLOAD_POS 7

#define VSOMEIP_REGISTER_APPLICATION 0x00
#define VSOMEIP_DEREGISTER_APPLICATION 0x01
#define VSOMEIP_APPLICATION_LOST 0x02
#define VSOMEIP_ROUTING_INFO 0x03
#define VSOMEIP_REGISTERED_ACK 0x04

#define VSOMEIP_PING 0x0E
#define VSOMEIP_PONG 0x0F

#define VSOMEIP_OFFER_SERVICE 0x10
#define VSOMEIP_STOP_OFFER_SERVICE 0x11
#define VSOMEIP_SUBSCRIBE 0x12
#define VSOMEIP_UNSUBSCRIBE 0x13
#define VSOMEIP_REQUEST_SERVICE 0x14
#define VSOMEIP_RELEASE_SERVICE 0x15
#define VSOMEIP_SUBSCRIBE_NACK 0x16
#define VSOMEIP_SUBSCRIBE_ACK 0x17

#define VSOMEIP_SEND 0x18
#define VSOMEIP_NOTIFY 0x19
#define VSOMEIP_NOTIFY_ONE 0x1A

#define VSOMEIP_REGISTER_EVENT 0x1B
#define VSOMEIP_UNREGISTER_EVENT 0x1C
#define VSOMEIP_ID_RESPONSE 0x1D
#define VSOMEIP_ID_REQUEST 0x1E
#define VSOMEIP_OFFERED_SERVICES_REQUEST 0x1F
#define VSOMEIP_OFFERED_SERVICES_RESPONSE 0x20
#define VSOMEIP_UNSUBSCRIBE_ACK 0x21
#define VSOMEIP_RESEND_PROVIDED_EVENTS 0x22

#define VSOMEIP_UPDATE_SECURITY_POLICY 0x23
#define VSOMEIP_UPDATE_SECURITY_POLICY_RESPONSE 0x24
#define VSOMEIP_REMOVE_SECURITY_POLICY 0x25
#define VSOMEIP_REMOVE_SECURITY_POLICY_RESPONSE 0x26
#define VSOMEIP_UPDATE_SECURITY_CREDENTIALS 0x27
#define VSOMEIP_DISTRIBUTE_SECURITY_POLICIES 0x28

#define VSOMEIP_SEND_COMMAND_SIZE 14
#define VSOMEIP_SEND_COMMAND_INSTANCE_POS_MIN 7
#define VSOMEIP_SEND_COMMAND_INSTANCE_POS_MAX 8
#define VSOMEIP_SEND_COMMAND_FLUSH_POS 9
#define VSOMEIP_SEND_COMMAND_RELIABLE_POS 10
#define VSOMEIP_SEND_COMMAND_VALID_CRC_POS 11
#define VSOMEIP_SEND_COMMAND_DST_CLIENT_POS_MIN 12
#define VSOMEIP_SEND_COMMAND_DST_CLIENT_POS_MAX 13
#define VSOMEIP_SEND_COMMAND_PAYLOAD_POS 14

#define VSOMEIP_OFFER_SERVICE_COMMAND_SIZE 16
#define VSOMEIP_REQUEST_SERVICE_COMMAND_SIZE 17
#define VSOMEIP_RELEASE_SERVICE_COMMAND_SIZE 11
#define VSOMEIP_STOP_OFFER_SERVICE_COMMAND_SIZE 16
#define VSOMEIP_SUBSCRIBE_COMMAND_SIZE 19
#define VSOMEIP_SUBSCRIBE_ACK_COMMAND_SIZE 19
#define VSOMEIP_SUBSCRIBE_NACK_COMMAND_SIZE 19
#define VSOMEIP_UNSUBSCRIBE_COMMAND_SIZE 17
#define VSOMEIP_UNSUBSCRIBE_ACK_COMMAND_SIZE 15
#define VSOMEIP_REGISTER_EVENT_COMMAND_SIZE 15
#define VSOMEIP_UNREGISTER_EVENT_COMMAND_SIZE 14
#define VSOMEIP_ID_RESPONSE_COMMAND_SIZE 12
#define VSOMEIP_ID_REQUEST_COMMAND_SIZE 13
#define VSOMEIP_OFFERED_SERVICES_COMMAND_SIZE 8
#define VSOMEIP_RESEND_PROVIDED_EVENTS_COMMAND_SIZE 11
#define VSOMEIP_REMOVE_SECURITY_POLICY_COMMAND_SIZE 19
#define VSOMEIP_UPDATE_SECURITY_POLICY_RESPONSE_COMMAND_SIZE 11
#define VSOMEIP_REMOVE_SECURITY_POLICY_RESPONSE_COMMAND_SIZE 11
#define VSOMEIP_PING_COMMAND_SIZE 7
#define VSOMEIP_PONG_COMMAND_SIZE 7
#define VSOMEIP_REGISTER_APPLICATION_COMMAND_SIZE 7
#define VSOMEIP_DEREGISTER_APPLICATION_COMMAND_SIZE 7
#define VSOMEIP_REGISTERED_ACK_COMMAND_SIZE 7

typedef enum
{
  SUBSCRIPTION_ACKNOWLEDGED,
  SUBSCRIPTION_NOT_ACKNOWLEDGED,
  IS_SUBSCRIBING
} subscription_state_e;

typedef enum
{
  RIE_ADD_CLIENT = 0x0,
  RIE_ADD_SERVICE_INSTANCE = 0x1,
  RIE_DEL_SERVICE_INSTANCE = 0x2,
  RIE_DEL_CLIENT = 0x3,
} routing_info_entry_e;

// enum class state_type_e : uint8_t {
//     ST_REGISTERED = 0x0,
//     ST_DEREGISTERED = 0x1
// };

enum message_type_e
{
  MT_REQUEST = 0x00,
  MT_REQUEST_NO_RETURN = 0x01,
  MT_NOTIFICATION = 0x02,
  MT_REQUEST_ACK = 0x40,
  MT_REQUEST_NO_RETURN_ACK = 0x41,
  MT_NOTIFICATION_ACK = 0x42,
  MT_RESPONSE = 0x80,
  MT_ERROR = 0x81,
  MT_RESPONSE_ACK = 0xC0,
  MT_ERROR_ACK = 0xC1,
  MT_UNKNOWN = 0xFF
};

// // SIP_RPC_371
// enum class return_code_e : uint8_t {
//     E_OK = 0x00,
//     E_NOT_OK = 0x01,
//     E_UNKNOWN_SERVICE = 0x02,
//     E_UNKNOWN_METHOD = 0x03,
//     E_NOT_READY = 0x04,
//     E_NOT_REACHABLE = 0x05,
//     E_TIMEOUT = 0x06,
//     E_WRONG_PROTOCOL_VERSION = 0x07,
//     E_WRONG_INTERFACE_VERSION = 0x08,
//     E_MALFORMED_MESSAGE = 0x09,
//     E_WRONG_MESSAGE_TYPE = 0xA,
//     E_UNKNOWN = 0xFF
// };

typedef uint8_t subscription_type_e;
enum
{
  SU_RELIABLE_AND_UNRELIABLE = 0x00,
  SU_PREFER_UNRELIABLE = 0x01,
  SU_PREFER_RELIABLE = 0x02,
  SU_UNRELIABLE = 0x03,
  SU_RELIABLE = 0x04,
};

// enum class routing_state_e : uint8_t {
//     RS_RUNNING = 0x00,
//     RS_SUSPENDED = 0x01,
//     RS_RESUMED = 0x02,
//     RS_SHUTDOWN = 0x03,
//     RS_DIAGNOSIS = 0x04,
//     RS_UNKNOWN = 0xFF
// };

// enum class offer_type_e : uint8_t {
//     OT_LOCAL = 0x00,
//     OT_REMOTE = 0x01,
//     OT_ALL = 0x02,
// };

// enum class security_update_state_e : uint8_t {
//     SU_SUCCESS = 0x00,
//     SU_NOT_ALLOWED = 0x01,
//     SU_UNKNOWN_USER_ID = 0x02,
//     SU_INVALID_FORMAT = 0x03
// };

#pragma pack(push, 1)

typedef struct
{
  uint32_t e;
  uint32_t size;
} routing_info_t;

typedef struct
{
  service_t service_;
  instance_t instance_;
  major_version_t major_;
  minor_version_t minor_;
  uint8_t use_exclusive_proxy_; // only used for requests!
} service_data_t;

typedef struct
{
  service_t service;
  instance_t instance;
  eventgroup_t eventgroup;
  major_version_t major;
  event_t event;
  uint8_t subscription_type;
  pending_subscription_id_t pending_subscription_id;
} subscribe_cmd_t;

typedef struct
{
  service_t service;
  instance_t instance;
  eventgroup_t eventgroup;
  client_t subscriber;
  event_t event;
} subscribe_ack_t;

typedef struct
{
  service_t service;
  method_t method;
  uint32_t length;
  client_t src_client;
  session_t session;
  protocol_version_t protocol_version;
  interface_version_t interface_version;
  uint8_t message_type;
  uint8_t return_code;
} command_header_t;

typedef struct
{
  instance_t instance;
  bool_t flush;
  bool_t realiable;
  bool_t is_valid_crc;
  client_t dst_client;
} send_cmd_t;

typedef struct
{
  uint8_t cmd_id;
  uint16_t client_id;
  uint32_t payload_size;
} packet_header_t;

#pragma pack(pop)

int send_packet(int sockfd, packet_header_t* header, void* payload);
int open_vsomeipd(int appid);

#define _SETTYPE(node, type) ((node)->extra = type)
#define _SETVALUE(node, value) ((node)->_private = (void*)(uintptr_t)(value))
#define _CHECKTYPE(node, type) ((node)->extra == type)
#define _GETVALUE(node) ((uintptr_t)(node)->_private)

#endif
