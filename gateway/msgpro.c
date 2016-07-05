/*****************************************************************************
Copyright:     2015-2016
File name:     msgpro.c
Description:	 ���ļ����ڶԽ����Լ��̼߳����Ϣ���̴���
Author:        wangjianjun
Version:       v1.0
Date:          2016-05-31
History:       2016-05-31 v1.0
*****************************************************************************/
#include "msgpro.h"

/*************************************************
Function:       int msgQCreate(int msgkey)
Description:    ��Ϣ���д���
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgkey-��Ϣ���б�ʶ
Output:         ��Ϣ���к�
Return:         ��Output��ͬ
Others:         NONE
*************************************************/
int msgQCreate(int msgkey)
{
    int msgid = -1;

    //������Ϣ����
    msgid = msgget((key_t)msgkey, 0666 | IPC_CREAT);
    if(msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
    }
    return msgid;
}

/*************************************************
Function:       void msgQsend(int msgid, char* sendbuf, int msglen)
Description:    ��Ϣ���ͺ���
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgid-��Ϣ���к�
                sendbuf-������Ϣ����
				  msglen-������Ϣ����
Output:         ��Ϣ���к�
Return:         ��Output��ͬ
Others:         NONE
*************************************************/
void msgQsend(int msgid, char* sendbuf, int msglen)
{
    struct msg_st data;
    data.msg_type = MSG_GATEWAY_DAEMOM;//gateway����daemo����Ϣ����
    memcpy(data.text, sendbuf, msglen);
    //����з�������
    if(msgsnd(msgid, (void*)&data, msglen, IPC_NOWAIT) == -1)
    {
        fprintf(stderr, "msgsnd failed : %s\n", strerror(errno));
    }
}

/*************************************************
Function:       struct msg_communicate* msgQrecv(int msgid)
Description:    ��Ϣ���е���Ϣ����
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          msgid-��Ϣ���к�
Output:         ���յ�����Ϣ
Return:         ��Output��ͬ
Others:         NONE
*************************************************/
struct msg_communicate* msgQrecv(int msgid)
{
    struct msg_st data;
    char *recvbuf = NULL;//������Ϣ�Ļ���
    int recvlen = 0;
    int msgtype = MSG_DAEMOM_GATEWAY;//daemo����gateway����Ϣ����
    struct msg_communicate *recvdata=NULL;
	//����msgrcv��ȡ��Ϣ�����е���Ϣ
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
    return recvdata;//��ת��֮�����Ϣָ�뷵��
}


/*************************************************
Function:       void msgprocess()
Description:    ��Ϣ���д���������
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
Return:         ��Output��ͬ
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
Description:    ��Ϣ���е���Ϣ����
Calls:          NONE
Called By:      msgpro.c(
                         void msgprocess()
                        )
Input:          recvdata-���յ�����Ϣָ��
Output:         NULL
Return:         ��Output��ͬ
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

