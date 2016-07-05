#ifndef MSGPRO_H_INCLUDED
#define MSGPRO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include "globalValue.h"
#include <string.h>
#include <unistd.h>

#define MSG_GATEWAY_DAEMOM 1
#define MSG_DAEMOM_GATEWAY 2

//消息队列接收消息结构体定义
struct msg_st
{
    long int msg_type;
    char text[BUFSIZ];
};

//进程间通信应用层消息结构体
struct msg_communicate
{
    int msgid;
    int msglen;
    char* data;
};

//发送消息列表
enum _SEND_MSGID_
{
    SEND_HAND = 0x1,
    SEND_HEART = 0x2,
    SEND_CONFFILE = 0x3,
    SEND_MAX
};

//接收消息列表
enum _RECV_MSGID_
{
    RECV_ANSWER = 0x1,
    RECV_MAX
};


//消息队列号列表
enum _COMNODE_
{
    GATEWAY_DAEMON,
    MSGID_MAX
};

int msgid[MSGID_MAX];

int msgQCreate(int msgkey);
void msgQsend(int msgid, char* sendbuf, int msglen);
struct msg_communicate* msgQrecv(int msgid);
void msgprocess();
void msgparse(struct msg_communicate *recvdata);

#endif // MSGPRO_H_INCLUDED

