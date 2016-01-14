//
// Created by xyy on 15-12-18.
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

uint32_t g_session_id = 0;
uint8_t g_display_type = 0;
uint8_t g_display_id = 0;

void init_link_msg(red_link_msg *link_msg, int session_ID, int channel_type,
                   int channel_ID,int client_comm_caps,int channel_spe_caps){
    link_msg->spice_magic[0] = 'R';
    link_msg->spice_magic[1] = 'E';
    link_msg->spice_magic[2] = 'D';
    link_msg->spice_magic[3] = 'Q';

    link_msg->major_version = 2;
    link_msg->minor_version = 2;
    link_msg->message_size = 26;
    link_msg->session_ID = session_ID;
    link_msg->channel_type = channel_type;
    link_msg->channel_ID = channel_ID;
    link_msg->num_com_caps = 1;
    link_msg->num_channel_caps = 1;
    link_msg->caps_offset = 18;
    link_msg->client_comm_caps = client_comm_caps;
    link_msg->channel_spe_caps = channel_spe_caps;
}

int send_pong_msg(int cfd, uint32_t ping_ID, uint64_t ping_timestamp){
    char *send_buf = (char *)malloc(sizeof(char)*200);
    spice_msg_miniheader msg_miniheader;
    msg_miniheader.type = 3;
    msg_miniheader.size = 12;

    memcpy(send_buf,&msg_miniheader,sizeof(spice_msg_miniheader));

    spice_msg_pong msg_pong;
    msg_pong.id = ping_ID;
    msg_pong.timestamp = ping_timestamp;

    *((uint32_t *)&send_buf[6]) = ping_ID;
    *((uint32_t *)&send_buf[10]) = ping_timestamp;

    int sendn = send(cfd,send_buf,18,0);
    if(sendn != 18){
        printf("send pong message error\n");
        exit(0);
    }

    return 0;

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

unsigned char *encrypt_password(unsigned char * pub_key_info, unsigned char *password){
    const unsigned char *pub_key = pub_key_info;
    int RSASize;
    RSA *rsa;

    unsigned char *encrypt_buf = (unsigned char*) malloc(128*sizeof(char));

    rsa = d2i_RSA_PUBKEY(NULL, &pub_key, 162);
    RSASize = RSA_size(rsa);

    RSA_public_encrypt(1,password,encrypt_buf,rsa,RSA_PKCS1_OAEP_PADDING);

    return encrypt_buf;
}

int make_stream_file(int stream_sequence_num, char *media_buf, int stream_size){
    FILE *fp = NULL;
    char *file_name = (char *)malloc(sizeof(char)*100);
//    sprintf(file_name,"%s%d","/home/sjtu/code/ClionProjects/client2/stream_data/",stream_sequence_num);
    sprintf(file_name,"%s%s","/home/xyy/spice_client/stream_data/","frame_data");
    printf("file name %s\n",file_name);

    if((fp = fopen(file_name,"ab+")) == NULL){
        printf("create file error");
        exit(0);
    }

    if((fwrite(&media_buf[12],1,stream_size,fp) != stream_size)){
        printf("write file error\n");
        exit(0);
    }
    fclose(fp);
    free(file_name);
    return 0;
}
