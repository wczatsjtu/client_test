//
// Created by sjtu on 15-12-21.
//

#ifndef CLIENT_TEST_MESSAGE_H
#define CLIENT_TEST_MESSAGE_H


#include <stdint-gcc.h>

extern int SESSIONID;

void init_clientlink_message(struct ClientLinkMessage *);
void sendpong(uint32_t cfd,uint32_t pingid, uint64_t timestamp);
int init_socket(char *ip_addr, int port);
int cal_time(uint64_t multimedia_time,uint64_t current_time,uint32_t body_size);


#pragma pack(1)
typedef struct ClientLinkMessage
{
    char SPICE_MAGIC[4];
    uint32_t Protocol_major_version;
    uint32_t Protocol_minor_version;
    uint32_t Message_size;
    uint32_t Session_ID;
    uint8_t channel_type;
    uint8_t channel_ID;
    uint32_t Number_of_common_capabilities;
    uint32_t Number_of_channel_capabilities;
    uint32_t Capabilities_offset;
    uint32_t client_Common_Capabilities;
    uint32_t client_channel_specific_capabilities;
}ClientLinkMessage;
#pragma pack()

#pragma pack(1)
typedef struct ServerLinkMessage
{
    char SPICE_MAGIC[4];
    uint32_t Protocol_major_version;
    uint32_t Protocol_minor_version;
    uint32_t Message_size;
    uint32_t Spice_error;
    uint8_t Publie_key[162];
    uint32_t Number_of_common_capabilities;
    uint32_t Number_of_channel_capabilities;
    uint32_t Capabilities_offset;
    uint32_t client_Common_Capabilities;
    uint32_t client_channel_specific_capabilities;
}ServerLinkMessage;
#pragma pack()

#pragma pack(1)
typedef struct SpiceLinkAuthMechanism
{
    uint32_t auth_mechanism;
}SpiceLinkAuthMechanism;
#pragma pack()

#pragma pack(1)
typedef struct ServerInitMessage
{
    uint16_t Message_type;
    uint32_t Message_body_size;
    uint32_t Session_ID;
    uint32_t Number_of_display_channels;
    uint32_t Supported_mouse_modes;
    uint32_t Current_mouse_mode;
    uint32_t Agent;
    uint32_t Agent_tokens;
    uint32_t Current_server_multimedia_time;
    uint32_t RAM_hint;
}ServerInitMessage ;
#pragma pack()

#pragma pack(1)
typedef struct ClientAttachChannels
{
    uint16_t Message_type = 104;
    uint32_t Message_body_size = 0;
}ClientAttachChannels ;
#pragma pack()


#pragma pack(1)
typedef struct ServerPing
{
    uint16_t Message_type;
    uint16_t Message_body_size;
    uint32_t Ping_ID;
    uint64_t time;
}ServerPing ;
#pragma pack()


#pragma pack(1)
typedef struct ClientPong
{
    uint16_t Message_type;
    uint32_t Message_body_size;
    uint32_t Ping_ID;
    uint64_t time;
}ClientPong ;
#pragma pack()


#pragma pack(1)
typedef struct ClientDisplayInit
{
    uint16_t Message_type = 101;
    uint32_t Message_body_size = 14;
    uint8_t  Cache_id = 1;
    uint64_t Cache_size = 20971520;
    uint8_t  Dictionary_id = 1;
    uint32_t Window_size = 6290432;
}ClientDisplayInit ;
#pragma pack()


#pragma pack(1)
typedef struct ClientAckSync
{
    uint16_t Message_type = 1;
    uint32_t Message_body_size = 4;
    uint32_t Ack_generation =1 ;
}ClientAckSync ;
#pragma pack()

#pragma pack(1)
typedef struct ClientAck
{
    uint16_t Message_type = 2;
    uint32_t Message_body_size = 0;
}ClientAck ;
#pragma pack()








#endif //CLIENT_TEST_MESSAGE_H