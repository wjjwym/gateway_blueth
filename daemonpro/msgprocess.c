/*****************************************************************************
Copyright:     2015-2016
File name:     msgprocess.c
Description:
Author:        wangjianjun
Version:       v1.0
Date:          2016-05-31
History:       2016-05-31 v1.0
*****************************************************************************/
#include "msgprocess.h"


/*************************************************
Function:       int msgQCreate(int msgkey)
Description:    创建消息队列与其它进程进行通信
Calls:          NONE
Called By:      msgprocess.c(
                      msgprocess(void)
                      )
Input:          消息队列类型号
Output:         NONE
Return:         msgid：消息队列ID
Others:         NONE
*************************************************/
int msgQCreate(int msgkey)
{
    int msgid = -1;

    //根据msgkey建立消息队列
    msgid = msgget((key_t)msgkey, 0666 | IPC_CREAT);
    if(msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
    }
    return msgid;
}

/*************************************************
Function:       void msgQsend(int msgid, char* sendbuf, int msglen)
Description:    消息队列发送函数
Calls:          NONE
Called By:      msgprocess.c(
                      msgprocess(void)
                      )
Input:          int msgid：消息队列ID
                char* sendbuf：发送缓存区
                int msglen：消息长度
Output:         发送消息内容
Return:         NONE
Others:         NONE
*************************************************/
void msgQsend(int msgid, char* sendbuf, int msglen)
{
    struct msg_st data;
    data.msg_type = MSG_DAEMOM_GATEWAY;//类型为发送到GATEWAY进程到消息
    strcpy(data.text, sendbuf);
    //向队列发送数据
    if(msgsnd(msgid, (void*)&data, sizeof(data), 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
    }
}


/*************************************************
Function:       struct msg_communicate* msgQrecv(int msgid)
Description:    消息队列接收函数
Calls:          NONE
Called By:      msgprocess.c(
                      msgprocess(void)
                      )
Input:          int msgid：消息队列ID
Output:         接收消息内容
Return:         struct msg_communicate*：接收的消息内容
Others:         NONE
*************************************************/
struct msg_communicate* msgQrecv(int msgid)
{
    struct msg_st data;
    char *recvbuf = NULL;
    int recvlen = 0;
    int msgtype = MSG_GATEWAY_DAEMOM;
    struct msg_communicate *recvdata = NULL;

    recvlen = msgrcv(msgid, &data, BUFSIZ, msgtype, IPC_NOWAIT);
    if( 0>recvlen )
    {//未接收到消息
        //printf("@\n");
        return NULL;
    }
    else if(0==recvlen)
    {//接收消息错误
        printf("recv msg error = %d\n",recvlen);
        return NULL;
    }
    else
    {
       printf("recv msg len = %d\n",recvlen);
    }
    recvbuf = malloc(recvlen);
    memcpy(recvbuf, data.text, recvlen);
    //remember to free
    recvdata = (struct msg_communicate *)recvbuf;
    recvdata->data = recvbuf + sizeof(recvdata->msgid) + sizeof(recvdata->msglen);

    return recvdata;
}


/*************************************************
Function:       void msgprocess()
Description:    消息处理主函数
Calls:          msgprocess.c(
                      int msgQCreate(int msgkey);
                      struct msg_communicate* msgQrecv(int msgid);
                      )
Called By:      main.c
                      main();
                      )
Input:          NONE
Output:         接收消息内容
Return:         NONE
Others:         NONE
*************************************************/
void msgprocess()
{
    int msgkey = 0x1122;
    int msgid = 0;
    struct msg_communicate *recvdata;
    msgid = msgQCreate(msgkey);//创建消息队列

    while(1)
    {
        //接收消息
        recvdata = msgQrecv(msgid);

        //解析接收到的消息
        if( NULL!=recvdata )
        {
            msgparse(recvdata);
            free(recvdata);
            recvdata=NULL;
        }

        usleep(10*1000);//10ms
    }
}

/*************************************************
Function:       void msgparse(struct msg_communicate *recvdata)
Description:    消息解析函数
Calls:          watchdog.c(
                      void recvheart_process(char *msgdata, int msglen)
                      )
                encryption.c(
                      void process_configfile(const char *p_conf_file, int filelen)
                      )
Called By:       msgprocess.c(
                      msgprocess();
                      )
Input:          struct msg_communicate *recvdata:接收到的消息
Output:         解析完成到数据内容
Return:         NONE
Others:         NONE
*************************************************/
void msgparse(struct msg_communicate *recvdata)
{
    switch (recvdata->msgid)
    {
        case RECV_HAND:

            break;
        case RECV_HEART:
            recvheart_process(recvdata->data, recvdata->msglen);
            printf("recv gateway heartbeat\n");
            break;
        case RECV_CONFFILE:
            printf("recv gateway configfile\n");
            process_configfile(recvdata->data, recvdata->msglen);
            break;
    }
}
