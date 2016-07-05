/*****************************************************************************
Copyright:     2015-2016
File name:     msgpro.c
Description:	 本文件用于对进程以及线程间的消息进程处理
Author:        wangjianjun
Version:       v1.0
Date:          2016-05-31
History:       2016-05-31 v1.0
*****************************************************************************/
#include "msgpro.h"

/*************************************************
Function:       int msgQCreate(int msgkey)
Description:    消息队列创建
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgkey-消息队列标识
Output:         消息队列号
Return:         与Output相同
Others:         NONE
*************************************************/
int msgQCreate(int msgkey)
{
    int msgid = -1;

    //建立消息队列
    msgid = msgget((key_t)msgkey, 0666 | IPC_CREAT);
    if(msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
    }
    return msgid;
}

/*************************************************
Function:       void msgQsend(int msgid, char* sendbuf, int msglen)
Description:    消息发送函数
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgid-消息队列号
                sendbuf-发送消息缓存
				  msglen-发送消息长度
Output:         消息队列号
Return:         与Output相同
Others:         NONE
*************************************************/
void msgQsend(int msgid, char* sendbuf, int msglen)
{
    struct msg_st data;
    data.msg_type = MSG_GATEWAY_DAEMOM;//gateway发向daemo的消息类型
    memcpy(data.text, sendbuf, msglen);
    //向队列发送数据
    if(msgsnd(msgid, (void*)&data, msglen, IPC_NOWAIT) == -1)
    {
        fprintf(stderr, "msgsnd failed : %s\n", strerror(errno));
    }
}

/*************************************************
Function:       struct msg_communicate* msgQrecv(int msgid)
Description:    消息队列的消息接收
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgid-消息队列号
Output:         接收到的消息
Return:         与Output相同
Others:         NONE
*************************************************/
struct msg_communicate* msgQrecv(int msgid)
{
    struct msg_st data;
    char *recvbuf = NULL;//接收消息的缓存
    int recvlen = 0;
    int msgtype = MSG_DAEMOM_GATEWAY;//daemo发向gateway的消息类型
    struct msg_communicate *recvdata=NULL;
	//调用msgrcv读取消息队列中的消息
    recvlen = msgrcv(msgid, &data, BUFSIZ, msgtype, IPC_NOWAIT);
    if( 0>recvlen )
    {
        //printf(".");
        return NULL;
    }
    else if(0==recvlen)
    {
        printf("recv msg error = %d\n",recvlen);
        return NULL;
    }
    else
    {
       printf("recv msg len = %d\n",recvlen);
    }

    recvbuf = malloc(recvlen);
    memcpy(recvbuf, data.text, recvlen);
    //remember to free recvdata
    recvdata = (struct msg_communicate *)recvbuf;
    return recvdata;//将转换之后的消息指针返回
}


/*************************************************
Function:       void msgprocess()
Description:    消息队列处理主函数
Calls:          msgpro.c(
                         void msgprocess()int msgQCreate(int msgkey)
							 struct msg_communicate* msgQrecv(int msgid)
							 void msgparse(struct msg_communicate *recvdata)
                        )
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          NULL
Output:         NULL
Return:         与Output相同
Others:         NONE
*************************************************/
void msgprocess()
{
    int msgkey = 0x1122;
    struct msg_communicate *recvdata;

    msgid[GATEWAY_DAEMON] = msgQCreate(msgkey);

    while(1)
    {
        //receive data from gateway
        recvdata = msgQrecv(msgid[GATEWAY_DAEMON]);

        //parse data
        if( NULL!=recvdata )
        {
            msgparse(recvdata);
        }

        usleep(1000*1000);//1s
    }
}

/*************************************************
Function:       void msgparse(struct msg_communicate *recvdata)
Description:    消息队列的消息接收
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          recvdata-接收到的消息指针
Output:         NULL
Return:         与Output相同
Others:         NONE
*************************************************/
void msgparse(struct msg_communicate *recvdata)
{
    switch (recvdata->msgid)
    {
        case RECV_ANSWER:
            break;
    }
    free(recvdata);
}

