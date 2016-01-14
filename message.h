//
// Created by xyy on 15-12-18.
//

#ifndef CLIENT_MESSAGE_H
#define CLIENT_MESSAGE_H

#include<stdint.h>

#define RED_TICKET_PUBKEY_BYTES 162

extern uint32_t g_session_id;
extern uint8_t g_display_type;
extern uint8_t g_display_id;

#pragma pack(1)
typedef struct red_link_msg{
    char spice_magic[4];
    int major_version;
    int minor_version;
    int message_size;
    int session_ID;
    char channel_type;
    char channel_ID;
    int num_com_caps;
    int num_channel_caps;
    int caps_offset;
    int client_comm_caps;
    int channel_spe_caps;
} red_link_msg;
#pragma pack()

#pragma pack(1)
typedef struct red_link_reply{
    char spice_msgic[4];
    int major_version;
    int minor_version;
    int message_size;
    int error;
    unsigned char pub_key[RED_TICKET_PUBKEY_BYTES];
    int num_com_caps;
    int num_channel_caps;
    int caps_offset;
    int comm_caps;
    int channel_caps;
} red_link_reply;
#pragma pack()

#pragma pack(2)
typedef struct spice_msg_miniheader{
    uint16_t type;
    uint32_t size;
}spice_msg_miniheader;
#pragma pack()

//server ping
//type:4 size:12
typedef struct spice_msg_ping{
    uint32_t id;
    uint64_t timestamp;
    void *data;
    uint32_t data_len;
}spice_msg_ping;

//client pong
//type:3 size:12
typedef struct spice_msg_pong{
    uint32_t id;
    uint64_t timestamp;
}spice_msg_pong;

typedef struct red_channel_id{
    uint8_t type;
    uint8_t channel_id;
}red_channel_id;

//server init
//type:103 size:32
typedef struct red_main_init{
    uint32_t session_id;
    uint32_t display_channel_num;
    uint32_t supported_mouse_modes;
    uint32_t curr_mouse_mode;
    uint32_t agent_connected;
    uint32_t agent_tokens;
    uint32_t multi_media_time;
    uint32_t ram_hint;
}red_main_init;

//server channel list
//type:104 size:4+2*channel_number
typedef struct red_channels_list{
    uint32_t num_of_channels;
    red_channel_id* red_channel;
}red_channels_list;

//client display init
//type:101 size:14
#pragma pack(1)
typedef struct redc_display_init{
    uint8_t cache_id;
    uint64_t cache_size;
    uint8_t glz_dictionary_id;
    uint32_t dictionary_win_size;
}redc_display_init;
#pragma pack()
//client attach channels
//type:102 size:0
typedef spice_msg_miniheader client_attach_channels;

//serer inval all palettes
//type:108 size:0
typedef spice_msg_miniheader red_inval_all_palette;

//red display mark
//type:102 size:0
typedef spice_msg_miniheader red_display_mark;

//server set ack
//type:3 size:8
typedef struct red_set_ack{
    uint32_t set_ack_gen;
    uint32_t set_ack_win;
}red_set_ack;

//client ack sync
//type:1 size:4
typedef struct redc_ack_sync{
    uint32_t set_ack_gen;
}redc_ack_sync;



//server stream create
//type:122 size:51
typedef struct red_stream_create{
    uint32_t surface_id;
    uint32_t stream_id;
    uint8_t stream_flags;
    uint8_t stream_codec_type;
    uint64_t stamp;
    uint32_t stream_width;
    uint32_t stream_height;
    uint32_t stream_source_width;
    uint32_t stream_source_height;
    uint32_t rect_left;
    uint32_t rect_top;
    uint32_t rect_right;
    uint32_t rect_bottom;
    uint8_t  vlip_type;
}red_stream_init;


typedef struct client_ack{
    uint16_t message_type = 2;
    uint32_t bodysize = 0;
}client_ack;

void init_link_msg(red_link_msg *link_msg, int session_ID, int channel_type,
                   int channel_ID,int client_comm_caps,int channel_spe_caps);
int send_pong_msg(int cfd, uint32_t ping_ID, uint64_t ping_timestamp);
//void make_link_msg(char **str, red_link_msg link_msg);
//void read_link_reply(red_link_reply *link_reply, char *buf);
int init_socket(char *ip_addr, int port);
unsigned char *encrypt_password(unsigned char * pubKeyInfo, unsigned char *password);
int make_stream_file(int stream_sequence_num, char *media_buf, int stream_size);

#endif //CLIENT_MESSAGE_H
