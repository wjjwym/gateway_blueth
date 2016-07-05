#ifndef MSGPROCESS_H_INCLUDED
#define MSGPROCESS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "watchdog_pro.h"

#define MSG_GATEWAY_DAEMOM 1
#define MSG_DAEMOM_GATEWAY 2

struct msg_st
{
    long int msg_type;
    char text[BUFSIZ];
};


struct msg_communicate
{
    int msgid;
    int msglen;
    char* data;
};


enum _RECV_MSGID_
{
    RECV_HAND = 0x1,
    RECV_HEART = 0x2,
    RECV_CONFFILE = 0x3,
    RECV_MAX
};


enum _SEND_MSGID_
{
    SEND_ANSWER = 0x1,
    SEND_MAX
};

int msgQCreate(int msgkey);
void msgQsend(int msgid, char* sendbuf, int msglen);
struct msg_communicate* msgQrecv(int msgid);
void msgprocess();
void msgparse(struct msg_communicate *recvdata);

#endif // MSGPROCESS_H_INCLUDED
