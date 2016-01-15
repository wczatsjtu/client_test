#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/x509.h>
#include "message.h"
#include <pthread.h>
#include <sys/time.h>


using namespace std;

void *display_channel_handler(void *arg);



int main( int argc, char** argv )
{
    struct sockaddr_in server_addr_in;
    unsigned char rec_buf[1500];
    char *send_buf = (char *)malloc(42*sizeof(char));
    uint32_t cfd;
    uint32_t port = 5910;
//    初始化client_link_message
    ClientLinkMessage clientLinkMessage;
    init_clientlink_message(&clientLinkMessage);
    memcpy(send_buf,&clientLinkMessage,sizeof(clientLinkMessage));


//初始化socket连接
    cfd = socket( AF_INET, SOCK_STREAM, 0 );

    bzero( &server_addr_in, sizeof(server_addr_in) );

    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton( AF_INET, "172.16.5.84", &server_addr_in.sin_addr );

    if(connect( cfd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in) )<0)
        printf("socket connecting failure");

//发送client_link消息
    if(send( cfd,send_buf,42,0 )>0)
        printf("1.client_link message sent\n");

    sleep(1);
//接受server_link消息
    ServerLinkMessage serverLinkMessage;
    int count = recv( cfd, rec_buf, 1500, 0 );
    if(count >0)
        printf("2.server_link message received,bytes count: %d \n",count);
    memcpy(&serverLinkMessage,rec_buf,202);
    printf("    Recive from server : %s \n", rec_buf);
//发送authentication selection method消息
//    int client_auth = 1;
//    if(send(cfd,&client_auth,4,0)>0)
//        printf("3.client authentification method selection message sent \n");
    SpiceLinkAuthMechanism client_auth;
    client_auth.auth_mechanism = 1;
    if(send(cfd,&client_auth,4,0)>0)
       printf("3.client authentification method selection message sent \n");




//提取公钥，客户端密码加密发送
    const unsigned char * tmp_key = (const unsigned char *)serverLinkMessage.Publie_key;

//    BIO *bio_key = BIO_new(BIO_s_mem());
//    EVP_PKEY *pub_key;
    int nRSASize;
    RSA *rsa;
    rsa = d2i_RSA_PUBKEY(NULL,&tmp_key,162);
    nRSASize = RSA_size(rsa);
//    printf("rsa size : %d\n",nRSASize);
    unsigned char *password=(unsigned char*)"";
    unsigned char encrypt_buf[128];
    RSA_public_encrypt(1,password,encrypt_buf,rsa,RSA_PKCS1_OAEP_PADDING);
    if(send(cfd,encrypt_buf,128,0)>0)
        printf("4.client ticket sent\n");

//    EVP_PKEY_free(pub_key);

//    BIO_free(bio_key);


//接受server ticket消息
    count=recv(cfd,rec_buf,4,0);
//    if(count>0)
//        printf("5.server ticket received,bytes:%d\n", count);
    if(*(uint32_t *)&rec_buf[0] == 0)
        printf("5.server ticket received\n");


    ClientAttachChannels clientAttachChannels;
    if(send(cfd,&clientAttachChannels,sizeof(clientAttachChannels),0) == 6)
        printf("client attach_channels message sent\n");

    //display channel
    pthread_t display_thread;
    int ret_thread;
    ret_thread = pthread_create(&display_thread,NULL,display_channel_handler,NULL);
    if(ret_thread != 0){
        printf("error in create display channel thread !\n");
    }else{
        printf("success in create display channel thread\n");
    }


    uint16_t message_type;
    uint32_t body_size;
    while(1)
    {
        count = recv(cfd,rec_buf,6,0);
        message_type = *(uint16_t *)&rec_buf[0];
        body_size = *(uint32_t *)&rec_buf[2];

        switch(message_type)
        {
            //Server NOTIFY
            case 7:
            {
                count = recv(cfd,rec_buf,body_size,0);
//              printf("Server NOTIFY message received\n");
                break;
            }
            //Server INIT
            case 103:
            {
                count = recv(cfd,rec_buf,body_size,0);
                printf("Server INIT message received, Session ID reserved\n");
                SESSIONID = *(uint32_t *)&rec_buf[0];
                break;
            }
            //Server Channels_LIST
            case 104:
            {
                count = recv(cfd,rec_buf,body_size,0);


                printf("Server channels_List message received\n");
                break;
            }
            // Server Mouse_MODE
            case 105:
            {
                count = recv(cfd,rec_buf,body_size,0);
//                printf("Server MOUSE MODE message received\n");
                break;
            }

            // Server AGENT_DISCONNECTED
            case 108:
            {
                count = recv(cfd,rec_buf,body_size,0);
 //             printf("server AGENT_DISCONNECTED message received\n");
                break;

            }
            //Server VM_UUID
            case 114:
            {
                count = recv(cfd,rec_buf,body_size,0);
//                printf("Server VM_UUID message received\n");
                break;
            }
            //Server PING
            case 4:
            {
                uint32_t pingid;
                uint64_t timestamp;
                if(body_size == 12)
                {
                    count = recv(cfd,rec_buf,body_size,0);
                    printf("tiny ping message received\n");
                    pingid = *(uint32_t *)&rec_buf[0];
                    timestamp = *(uint32_t *)&rec_buf[4];
                    sendpong(cfd,pingid,timestamp);
                }
                if(body_size == 256012)
                {
                    int i;
                    recv(cfd,rec_buf,1500,0);
                    pingid = *(uint32_t *)&rec_buf[0];
                    timestamp = *(uint32_t *)&rec_buf[4];
                    int circular = body_size/1500;
                    int last_packet_size = body_size%1500;
                    printf("circular = %d   last_packet_size = %d\n",circular,last_packet_size);

                    for(i=0;i<circular-1;i++)
                    {
                        recv(cfd,rec_buf,150,0);
                    }
                    recv(cfd,rec_buf,last_packet_size,0);
                    printf("big ping message received\n");
                    sendpong(cfd,pingid,timestamp);
//                    sleep(1);
                }
                break;
            }
            default:
            {
                printf("unhandled message received,message type:%d  message size:%d \n", message_type,body_size);
//                sleep(1);
                recv(cfd,rec_buf,1500,0);
                break;
            }
        }

    }


    close(cfd);
    return 0;
}


void *display_channel_handler(void *arg)
{
    printf("this is a message from display channel!!!\n");
//    struct sockaddr_in server_addr_in;
//    unsigned char rec_buf[1500];
//    char *send_buf = (char *)malloc(42*sizeof(char));
//    uint32_t cfd;
//    uint32_t port = 5910;
//
//
//    bzero( &server_addr_in, sizeof(server_addr_in) );
//
//    server_addr_in.sin_family = AF_INET;
//    server_addr_in.sin_port = htons(port);
//    inet_pton( AF_INET, "127.0.0.1", &server_addr_in.sin_addr );
//
//    if(connect( cfd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in) )<0)
//        printf("    display channel thread socket connecting failure\n");
    char send_buf[300];
    char rec_buf[1500];
    char media_buf[100000];
    uint16_t message_type;
    uint32_t body_size;

    int cfd;
    char *ip_addr = (char *)"172.16.5.84";
    int port = 5910;

    uint64_t stream_sequence = 0;

    uint16_t msg_type;
    uint32_t msg_size;
    uint32_t ping_ID;
    uint64_t ping_timestamp;

    cfd = init_socket(ip_addr,port);


    //初始化client_link_message
    ClientLinkMessage clientLinkMessage;
    init_clientlink_message(&clientLinkMessage);
    clientLinkMessage.Session_ID = SESSIONID;
    clientLinkMessage.channel_type = 2;
    clientLinkMessage.channel_ID = 0;
    clientLinkMessage.client_Common_Capabilities = 0x0d;
    clientLinkMessage.client_channel_specific_capabilities = 0x1f;

    memcpy(send_buf,&clientLinkMessage,sizeof(clientLinkMessage));

    //发送client_link消息
    if(send( cfd,send_buf,42,0 )>0)
        printf("    1.display client_link message sent\n");
    sleep(1);

    //接受server_link消息
    ServerLinkMessage serverLinkMessage;
    int count = recv( cfd, rec_buf, 1500, 0 );
    if(count >0)
        printf("    2.display server_link message received,bytes count: %d \n",count);
    memcpy(&serverLinkMessage,rec_buf,202);
    printf("    Recive from server : %s \n", rec_buf);
//发送authentication selection method消息
//    int client_auth = 1;
//    if(send(cfd,&client_auth,4,0)>0)
//        printf("3.client authentification method selection message sent \n");
    SpiceLinkAuthMechanism client_auth;
    client_auth.auth_mechanism = 1;
    if(send(cfd,&client_auth,4,0)>0)
        printf("    3.display client authentification method selection message sent \n");


    //提取公钥，客户端密码加密发送
    const unsigned char * tmp_key = (const unsigned char *)serverLinkMessage.Publie_key;

//    BIO *bio_key = BIO_new(BIO_s_mem());
//    EVP_PKEY *pub_key;
    int nRSASize;
    RSA *rsa;
    rsa = d2i_RSA_PUBKEY(NULL,&tmp_key,162);
    nRSASize = RSA_size(rsa);
//    printf("rsa size : %d\n",nRSASize);
    unsigned char *password=(unsigned char*)"";
    unsigned char encrypt_buf[128];
    RSA_public_encrypt(1,password,encrypt_buf,rsa,RSA_PKCS1_OAEP_PADDING);
    if(send(cfd,encrypt_buf,128,0)>0)
        printf("    4.display client ticket sent\n");

//    EVP_PKEY_free(pub_key);

//    BIO_free(bio_key);

    //接受server ticket消息
    count=recv(cfd,rec_buf,4,0);

    if(*(uint32_t *)&rec_buf[0] == 0)
        printf("    5.display server ticket received\n");

    //发送client display init
    ClientDisplayInit clientDisplayInit;
    send(cfd,&clientDisplayInit,sizeof(clientDisplayInit),0);

    uint32_t display_ack_window = 20;
    while(1)
    {
        count = recv(cfd,rec_buf,6,0);
        message_type = *(uint16_t *)&rec_buf[0];
        body_size = *(uint32_t *)&rec_buf[2];
        uint64_t multimedia_time;
        uint64_t current_time;
        struct timespec ts;
        struct timeval ts1;

        switch(message_type)
        {
            //Server SET_ACK
            case 3:
            {
                count = recv(cfd,rec_buf,body_size,0);
                display_ack_window = *(uint32_t *)&rec_buf[4];
                printf("    display channel SET_ACK message received,ack window = %d\n",display_ack_window);
                ClientAckSync clientAckSync;
                send(cfd,&clientAckSync,sizeof(clientAckSync),0);
                printf("    display channel ACK_SYNC message sent\n");
                break;
            }

            case 4:
            {
                uint32_t pingid;
                uint64_t timestamp;
                if(body_size == 12)
                {
                    count = recv(cfd,rec_buf,body_size,0);
                    printf("tiny ping message received\n");
                    pingid = *(uint32_t *)&rec_buf[0];
                    timestamp = *(uint32_t *)&rec_buf[4];
                    sendpong(cfd,pingid,timestamp);
                }
                if(body_size == 256012)
                {
                    int i;
                    recv(cfd,rec_buf,1500,0);
                    pingid = *(uint32_t *)&rec_buf[0];
                    timestamp = *(uint32_t *)&rec_buf[4];
                    int circular = body_size/1500;
                    int last_packet_size = body_size%1500;
                    printf("circular = %d   last_packet_size = %d\n",circular,last_packet_size);

                    for(i=0;i<circular-1;i++)
                    {
                        recv(cfd,rec_buf,150,0);
                    }
                    recv(cfd,rec_buf,last_packet_size,0);
                    printf("big ping message received\n");
                    sendpong(cfd,pingid,timestamp);
//                    sleep(1);
                }
                break;
            }

            //INVAL_ALL_PALETTES
            case 108:
            {
                count = recv(cfd,rec_buf,body_size,0);
                printf("    display channel INVAL_ALL_PALETTES message received\n");
                break;
            }
            //DRAW_SURFACE_CREATE
            case 314:
            {
                count = recv(cfd,rec_buf,body_size,0);
                printf("    display channel DRAW_SURFACE_CREATE message received\n");
                break;
            }
            //MONITORS_CONFIG
            case 317:
            {
                count = recv(cfd,rec_buf,body_size,0);
                printf("    display channel MONITORS_CONFIGE message received\n");
                break;
            }
            //MARK
            case 102:
            {
                count = recv(cfd, rec_buf, body_size, 0);
                printf("    display channel MARK message received\n");
                break;
            }
            //STREAM_CREATE
            case 122:
            {
                count = recv(cfd, rec_buf, body_size, 0);
                printf("    display channel STREAM_CREATE message received\n");
                break;
            }
            //STREAM_DATA
            case 123:
            {
                count = recv(cfd, media_buf, body_size, 0);

                multimedia_time = (uint64_t)*(uint32_t *)&media_buf[4];
                clock_gettime(CLOCK_MONOTONIC, &ts);
                gettimeofday(&ts1, NULL);
//                current_time = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
                current_time = (uint64_t)(ts1.tv_sec*1000 +ts1.tv_usec/1000);

//                printf("current time is : %ld    multimedia time is :%ld",current_time,multimedia_time);
                cal_time(multimedia_time,current_time,body_size);

//                printf("    display channel STREAM_DATA message received\n");
                stream_sequence++;
                if(stream_sequence%display_ack_window ==0)
                {
                    ClientAck clientAck;
                    send(cfd,&clientAck,sizeof(clientAck),0);
                }

                break;
            }
            case 125:
            {
                count = recv(cfd, rec_buf, body_size, 0);
                printf("    display channel DESTROY message received!!!!!!\n");
                break;
            }
            default:
            {
                printf("    display channel unhandled message received,message type:%d  message size:%d \n", message_type,body_size);
//                sleep(1);
//                recv(cfd,rec_buf,1500,0);
                break;
            }
        }



    }






}