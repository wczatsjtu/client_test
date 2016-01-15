//
// Created by sjtu on 15-12-23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ossl_typ.h>
#include <openssl/x509.h>

#include "message.h"

int SESSIONID;

uint64_t multimedia_time_unit[40];
uint64_t current_time_unit[40];
uint64_t body_size_unit[40];

//uint64_t multimedia_time_before;
//uint64_t current_time_before;
//uint64_t body_size_before;
int i = 0;

void init_clientlink_message(struct ClientLinkMessage *clientLinkMessage)
{
    clientLinkMessage->SPICE_MAGIC[0]= 'R';
    clientLinkMessage->SPICE_MAGIC[1]= 'E';
    clientLinkMessage->SPICE_MAGIC[2]= 'D';
    clientLinkMessage->SPICE_MAGIC[3]= 'Q';

    clientLinkMessage->Protocol_major_version=2;
    clientLinkMessage->Protocol_minor_version=2;
    clientLinkMessage->Message_size=26;
    clientLinkMessage->Session_ID= 0;
    clientLinkMessage->channel_type=1;
    clientLinkMessage->channel_ID=0;
    clientLinkMessage->Number_of_channel_capabilities=1;
    clientLinkMessage->Number_of_common_capabilities=1;
    clientLinkMessage->Capabilities_offset=18;
    clientLinkMessage->client_Common_Capabilities=13;
    clientLinkMessage->client_channel_specific_capabilities=15;
}

void sendpong(uint32_t cfd, uint32_t pingid, uint64_t timestamp)
{
    ClientPong clientPong;
    clientPong.Message_type = 3;
    clientPong.Message_body_size = 12;
    clientPong.Ping_ID = pingid;
    clientPong.time = timestamp;
    send(cfd,&clientPong,18,0);
    printf("pong message sent\n");
}


int init_socket(char *ip_addr, int port){
    int cfd;
    struct sockaddr_in server_addr_in;

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr_in, sizeof(server_addr_in));

    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET,ip_addr,&server_addr_in.sin_addr);

    if(connect(cfd, (struct sockaddr *)&server_addr_in,sizeof(server_addr_in))<0){
        printf("set socket error!\n");
    }

    return cfd;
}


int cal_time(uint64_t multimedia_time,uint64_t current_time,uint32_t body_size)
{
//    if(i == 0)
//    {
//        multimedia_time_before = multimedia_time;
//        current_time_before = current_time;
//        body_size_before = body_size;
//        i++;
//        return 0;
//    }
//    else
//    {
//        printf("i = %d",i);
//        uint64_t band_est;
//        printf("size difference: %ld Bytes    ",body_size-body_size_before);
//        printf("time difference: %ldms    ",current_time-current_time_before-multimedia_time+multimedia_time_before);
////        band_est = (body_size-body_size_before)*8*1000/(current_time-current_time_before-multimedia_time+multimedia_time_before);
////        printf("current bandwidth estimation: %ld kb/s   ",band_est);
//        current_time_before = current_time;
//        multimedia_time_before = multimedia_time;
//        body_size_before = body_size;
//        i++;
//    }
    if(i<40)
    {
        current_time_unit[i] = current_time;
        multimedia_time_unit[i] = multimedia_time;
        body_size_unit[i] = body_size;
        i++;
    }
    else
    {
        int64_t size_difference = 0;
        int64_t time_difference = 0;
        for(int j=0;j<20;j++)
        {
            size_difference += body_size_unit[j];
            time_difference += current_time_unit[j]- multimedia_time_unit[j];
        }
        for(int j=20;j<40;j++)
        {
            size_difference -= body_size_unit[j];
            time_difference -= current_time_unit[j]- multimedia_time_unit[j];
        }
        printf("size_difference = %ldBytes   time_difference = %ldms\n",size_difference,time_difference);
//        printf("estimated bandwidth:ld%kbps",size_difference/time_difference);
        i = 0;
        current_time_unit[i] = current_time;
        multimedia_time_unit[i] = multimedia_time;
        body_size_unit[i] = body_size;


    }
    return 0;
}