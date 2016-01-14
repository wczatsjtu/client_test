#include <iostream>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


#include <openssl/ssl.h>

#include "message.h"

//#define DEBUG_DEF
#define IP_OTHER

using namespace std;

void *display_channel_handler(void *arg);

int main() {
    char send_buf[500];
    unsigned char buf[1500];
    memset(buf,500,1);

    int cfd;

    char *ip_addr = (char *)"127.0.0.1";

#ifdef IP_OTHER
    ip_addr = (char *)"192.168.1.120";
#endif
    int port = 5910;

    cfd = init_socket(ip_addr,port);

//main client link message
    red_link_msg link_msg;
    red_link_reply link_reply;

    init_link_msg(&link_msg, 0, 1, 0, 0x0d, 0x0f);
    memcpy(send_buf,&link_msg,sizeof(red_link_msg));

    int sendn = send(cfd,send_buf,sizeof(red_link_msg),0);
    printf("send %d byte\n",sendn);

//main server link message
    sleep(1);
    int recn = recv(cfd,buf,sizeof(red_link_reply),0);
    if( recn < 0){
        printf("receive error!\n");
    }
    printf("receive %d byte\n",recn);

#ifdef DEBUG_DEF
//    printf("Receive from server : %s\n",buf);
/*
    printf("%c\n",buf[0]);
    printf("%c\n",buf[1]);
    printf("%c\n",buf[2]);
    printf("%c\n",buf[3]);

    int maj = *((int *)(&buf[4]));
    printf("maj = %d\n",maj);

    int min = *((int *)(&buf[8]));
    printf("min = %d\n",min);


    int size = *((int *)(&buf[12]));
    printf("size = %d\n",size);

    int err = *((int *)(&buf[16]));
    printf("err = %d\n",err);
*/
#endif

//main client authentication
    int client_auth = 1;
    send(cfd,&client_auth,4,0);

//main client ticket
    memcpy(&link_reply,buf,202);

    int nRSASize;
    RSA *rsa;
    unsigned char *encrypt_buf;

    encrypt_buf = encrypt_password(link_reply.pub_key,(unsigned char*)"");
    sendn = send(cfd,encrypt_buf,128,0);
    if(sendn != 128){
        printf("error in send client ticket!\n");
        exit(0);
    }
    free(encrypt_buf);

//main server ticket
    recv(cfd,buf,4,0);

    int link_result = *((uint32_t *)&buf[0]);
    if(link_result != 0){
        printf("link server error!\n");
        exit(0);
    }
    printf("link server success\n");

//main server init , main server VM_UUID, main server ping*2
    recn = recv(cfd,buf,38,0);
    recn += recv(cfd,&buf[recn],22,0);
    recn += recv(cfd,&buf[recn],18,0);
    recn += recv(cfd,&buf[recn],18,0);

    printf("receive %d bytes for server init\n",recn);

    uint16_t msg_type;
    uint32_t msg_size;
    uint32_t num_of_channels;
    uint32_t ping_ID;
    uint64_t ping_timestamp;

    msg_type = *((uint16_t *)&buf[0]);
    msg_size = *((uint32_t *)&buf[2]);

    g_session_id = *((uint32_t *)&buf[6]);
    num_of_channels = *((uint32_t *)&buf[10]);

    printf("msg type %d\n",msg_type);
    printf("msg size %d\n",msg_size);
    printf("msg session id %d\n",g_session_id);
    int offset = 0;
    uint8_t *msg_buf = (uint8_t *)malloc(500*sizeof(char));

    //server init length
    offset += 38;
    //server VM_UUID length
    offset += 22;
    memcpy(msg_buf,&buf[offset],18);

    msg_type = *((uint16_t *) &msg_buf[0]);
    ping_ID = *((uint32_t *) &msg_buf[6]);
    ping_timestamp = *((uint64_t *) &msg_buf[10]);
    if(msg_type == 4){
        send_pong_msg(cfd,ping_ID,ping_timestamp);
    }

    offset += 18;
    memcpy(msg_buf,&buf[offset],18);
    msg_type = *((uint16_t *) &msg_buf[0]);
    ping_ID = *((uint32_t *) &msg_buf[6]);
    ping_timestamp = *((uint64_t *) &msg_buf[10]);
    if(msg_type == 4){
        send_pong_msg(cfd,ping_ID,ping_timestamp);
    }

//client attach channels message
    client_attach_channels attach_channels;
    attach_channels.type = 104;
    attach_channels.size = 0;

    memcpy(send_buf,&attach_channels,sizeof(attach_channels));
    sendn = send(cfd,send_buf,sizeof(attach_channels),0);
    if(sendn != sizeof(attach_channels)){
        printf("send client attach channels error!\n");
        exit(0);
    }

//big ping packet
    recn = recv(cfd,buf,1500,0);
    printf("first packet size %d\n",recn);
    msg_type = *((uint16_t *) &buf[0]);
    msg_size = *((uint32_t *) &buf[2]);
    ping_ID = *((uint32_t *) &buf[6]);
    ping_timestamp = *((uint64_t *) &buf[10]);
    printf("ping packet size %d\n",msg_size);

    int count;
    int last_packet_size;
    for(count=0;count<((msg_size/1500)-1);count++){
        recn = recv(cfd,buf,1500,0);
    }

    last_packet_size = msg_size + 6 - (count+1)*1500;
    printf("last packet size = %d\n",last_packet_size);
    recn = recv(cfd,buf,last_packet_size,0);
    printf("last ping packet %d\n",recn);
    printf("ping packets count %d\n",count);
    if(msg_type == 4){
        send_pong_msg(cfd,ping_ID,ping_timestamp);
    }

//server channel list
    recn = recv(cfd,buf,6,0);
    msg_type = *((uint16_t *) &buf[0]);
    msg_size = *((uint32_t *) &buf[2]);
    if(msg_type == 104){
        recn = recv(cfd,&buf[6],10,0);
        if(recn != msg_size){
            printf("receive server channel list error!\n");
            exit(0);
        }
    }

    g_display_type = *((uint8_t *) &buf[12]);
    g_display_id = *((uint8_t *) &buf[13]);

    printf("display channel type %d and display channel id %d\n",g_display_type,g_display_id);

//display channel
    pthread_t display_thread;
    int ret_thread;
    void *retval;
    int tmp1;
    ret_thread = pthread_create(&display_thread,NULL,display_channel_handler,NULL);
    if(ret_thread != 0){
        printf("error in create display channel thread !\n");
    }else{
        printf("success in create display channel thread\n");
    }

    while(1){
        printf("main channel working\n");
        recn = recv(cfd,buf,6,0);
        msg_type = *((uint16_t *)&buf[0]);
        msg_size = *((uint32_t *)&buf[2]);

        if(msg_size != 0){
            recn = recv(cfd,&buf[6],msg_size,0);
            if(recn != msg_size) {
                printf("error in receive main message!\n");
                exit(0);
            }
        }

        switch(msg_type){
            //server ping
            case 4:{
                printf("receive main server ping\n");
                ping_ID = *((uint32_t *) &buf[6]);
                ping_timestamp = *((uint64_t *) &buf[10]);
                send_pong_msg(cfd,ping_ID,ping_timestamp);
                break;
            }
            case 59:{
                printf("receive serve NOTIFY");
                break;
            }
            case 104:{
                printf("receive server Channel_List\n");
                break;
            }
            case 105:{
                printf("receive server Mouse_MODE\n");
            }
            case 109:{
                printf("receive server AGENT_DATA\n");
                break;
            }
            default:{
                printf("receive main unhandled message type\n");
                break;
            }
        }

    }

/*    tmp1 = pthread_join(display_thread,&retval);
    if(tmp1 != 0){
        printf("can't join with display thread\n");
    }
    printf("display thread end\n");
*/
    close(cfd);
    return 0;
}


void *display_channel_handler(void *arg){
    printf("display thread begin\n");
    char send_buf[300];
    char rec_buf[1500];
    char media_buf[50000];

    int cfd;
    char *ip_addr = (char *)"127.0.0.1";

#ifdef IP_OTHER
    ip_addr = (char *)"192.168.1.120";
#endif
    int port = 5910;

    int stream_sequence = 0;
    uint32_t set_ack_window = 0;

    uint16_t msg_type;
    uint32_t msg_size;
    uint32_t ping_ID;
    uint64_t ping_timestamp;

    cfd = init_socket(ip_addr,port);

//display client link message
    red_link_msg display_link_msg;
    init_link_msg(&display_link_msg,g_session_id,g_display_type,g_display_id,0x0d,0x1f);

    memcpy(send_buf,&display_link_msg,sizeof(red_link_msg));

    int sendn = send(cfd,send_buf,sizeof(red_link_msg),0);

    if(sendn != sizeof(red_link_msg)){
        printf("error in send display client link message!\n");
        exit(0);
    }
    printf("send %d byte in display client link message\n",sendn);

//display server link message
    sleep(1);
    int recn = recv(cfd,rec_buf,sizeof(red_link_reply),0);
    printf("receive %d byte in display server link reply\n",recn);

//send client authentication method selection
    int client_auth = 1;
    send(cfd,&client_auth,4,0);

    red_link_reply display_link_reply;
    memcpy(&display_link_reply,rec_buf,sizeof(red_link_reply));

    if(display_link_reply.error != 0){
        printf("get a wrong display link reply error code!\n");
        exit(0);
    }

//display client ticket
    unsigned char *encrypt_buf;

    encrypt_buf = encrypt_password(display_link_reply.pub_key,(unsigned char*)"");
    sendn = send(cfd,encrypt_buf,128,0);
    if(sendn != 128){
        printf("error in send client ticket!\n");
        exit(0);
    }
    free(encrypt_buf);

//display server ticket
    recn = recv(cfd,rec_buf,4,0);
    int link_result = *((uint32_t *)&rec_buf[0]);
    if(recn != 4 || link_result != 0){
        printf("error in receiving server ticket!\n");
        exit(0);
    }
    printf("display channel link server success\n");

//display client init
    spice_msg_miniheader init_header;
    redc_display_init init_msg;

    init_header.type = 101;
    init_header.size = 14;

    init_msg.cache_id = 1;
    init_msg.cache_size = 0x0140;
    init_msg.glz_dictionary_id = 1;
    init_msg.dictionary_win_size = 0x005ffc00;

    memcpy(send_buf,&init_header,6);
    memcpy(&send_buf[6],&init_msg,14);

    sendn = send(cfd, send_buf, 20, 0);
    if(sendn != 20){
        printf("error in send display client init!\n");
        exit(0);
    }

//display server set_ack...
    while(1){
        recn = recv(cfd,rec_buf,6,0);
        msg_type = *((uint16_t *)&rec_buf[0]);
        msg_size = *((uint32_t *)&rec_buf[2]);

//stream data is handled in media buf
        if(msg_size != 0 && msg_type!=123){
            printf("msg_size :%d\n",msg_size);
            recn = recv(cfd,&rec_buf[6],msg_size,0);
            if(recn != msg_size){
                printf("error in receive display message!\n");
                exit(0);
            }
        }

        switch(msg_type){
            //server set ack
            case 3:{
                printf("receive display server SET_ACK\n");
                uint32_t ack_generation = *((uint32_t *)&rec_buf[6]);
                set_ack_window = *((uint32_t *)&rec_buf[10]);
                spice_msg_miniheader ack_sync_header;
                ack_sync_header.type = 1;
                ack_sync_header.size = 4;
                memcpy(send_buf,&ack_sync_header,6);
                *((uint32_t *)&send_buf[6]) = ack_generation;
                sendn = send(cfd,send_buf,10,0);
                if(sendn != 10){
                    printf("error in send display client ack sync!\n");
                    exit(0);
                }
                break;
            }
            //server ping
            case 4:{
                printf("receive display server ping\n");
                ping_ID = *((uint32_t *) &rec_buf[6]);
                ping_timestamp = *((uint64_t *) &rec_buf[10]);
                send_pong_msg(cfd,ping_ID,ping_timestamp);
                break;
            }
            //server mark
            case 102:{
                printf("receive display server mark\n");
                break;
            }
            //server inval_all_palette
            case 108: {
                printf("reveive display server INVAL_ALL_PALETTE\n");
                break;
            }
            //server stream_create
            case 122:{
                printf("receive display server STREAM_CREATE\n");
                break;
            }
            //server stream_data
            case 123:{
                printf("receive display server STREAM_DATA!\n");
                stream_sequence++;

                //send display client ack
                if(stream_sequence%set_ack_window == 0){
                    spice_msg_miniheader client_ack;
                    client_ack.type = 2;
                    client_ack.size = 0;
                    memcpy(send_buf,&client_ack,6);
                    sendn = send(cfd,send_buf,6,0);
                    if(sendn != 6){
                        printf("error in send display client ack!\n");
                    }
                    printf("send display client ack\n");
                }
                recn = recv(cfd,media_buf,msg_size,0);
                if(recn != msg_size){
                    printf("error in receive stream data!\n");
                    exit(0);
                }

                int stream_ID = *((uint32_t *)&media_buf[0]);
                int stream_data_size = *((uint32_t *)&media_buf[8]);
//                int stream_multimedia_time = *（）
                printf("stream ID %d the %dth stream data size is %d\n",stream_ID,stream_sequence,stream_data_size);


                make_stream_file(stream_sequence,media_buf,stream_data_size);
                break;
            }
            //server draw_surface_create
            case 314: {
                printf("receive display server DRAW_SURFACE_CREATE\n");
                break;
            }
            //server monitors_config
            case 317:{
                printf("receive display server MONITORS_CONFIG\n");
                break;
            }
            default: {
                printf("receive display unhandled message type!\n");
                break;
            }
        }

//        memset(rec_buf,0,1500);

    }

}
