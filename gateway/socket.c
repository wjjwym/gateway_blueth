/*****************************************************************************
Copyright:     2015-2016, FireWork.
File name:     socket.c
Description:   ���ļ�������JSON���ݰ��������йصĺ��������ڹ��ߺ����ļ�
Author:        ShaoYang Yu
Version:       v3.0
Date:          2016-05-15
History:       2016-04-26 v1.0
               2016-05-8  v2.0
               2016-05-11 v3.0
*****************************************************************************/

#include"socket.h"
#include"linux/tcp.h"
/*����������ַ�����*/
#define MAX 65535

/*������IP��ַ���˿ں�����*/
const char* IP = "121.42.196.94";
const int SERVERPORT = 10101;

unsigned char * raw_send_buffer_tick;   /*ԭ��tick���ݰ�Ԥ���ͻ�����*/
unsigned char * send_buffer_tick;   /*tick���ݰ����ͻ�����*/

unsigned char * raw_send_buffer = NULL;    /*ԭ�������ͻ�����*/
unsigned char raw_recv_buffer[MAX];    /*����Ԥ���ջ�����*/

unsigned char * send_buffer;    /*���ݷ��ͻ�����*/

static char netconnect_flg = 0;
/*************************************************
Function:       int create_socket()
Description:    ��ʼ��socket�����������������ͨ�����ͻ����JSON���ݰ�
Calls:          socket.c(
                        void send_tick(void*arg)
                        void send_json_packet(void *arg)
                        void recv_json_packet(void *arg)
                        )
Called By:      main.c(
                      int main()
                    )
Input:          NONE
Output:         �����׽��֣����Ͳ�����JSON���ݰ�
Return:         0
Others:         NONE
*************************************************/
int create_socket()
{
    int ser_socketfd = -1;	/*�����׽��ֱ�Ƿ�*/
    struct sockaddr_in remote_addr; 	/*����socket�ṹ��*/
    pthread_t send_socket_tick; /*��������tick�߳�������*/
    pthread_t send_socket;	/*���������߳�������*/
    pthread_t recv_socket;	/*���������߳�������*/
    int serverPort = 0;
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));	/*��ʼ���ṹ��*/
    remote_addr.sin_family = AF_INET;	/*����ʹ������IPv4Э��*/
    remote_addr.sin_addr.s_addr = inet_addr(m_configlist[CONF_SERVERIP].data);	  /*����IP��ַ*/
    serverPort = atoi(m_configlist[CONF_SERVERPORT].data);
    remote_addr.sin_port = htons(serverPort);	/*���÷������˿�*/

    struct sockaddr_in re_remote_addr;
    int re_namelen = 0;//getpeername return len

    while(1)/*������ѭ������ִ�йر��׽�������*/
    {
        delay_s(1);
        if(0==netconnect_flg)
        {//retry to connect server
            /*�����׽���*/
            if((ser_socketfd = socket(AF_INET, SOCK_STREAM, 0))<0)
            {
                perror("socket failed.");
                close(ser_socketfd);
                continue;
                //return 1;
            }

            /*���׽�����ṹ��������ӣ���������˷������������ź�*/
            if((connect(ser_socketfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))) < 0)
            {
                perror("connect failed.");
                close(ser_socketfd);
                continue;
                //return 1;
            }
            else
            {
                netconnect_flg = 1;//success to connect server
                unsigned long iMode = 1;
                ioctl(ser_socketfd, FIONBIO, &iMode);
            }

            /*�����׽���tick���ݷ����߳�*/
            if((pthread_create(&send_socket_tick, NULL, send_tick,(void*)ser_socketfd)) != 0)
            {
                perror("send thread create failed.");
                return 1;
            }

            /*�����׽������ݷ����߳�*/
            if((pthread_create(&send_socket, NULL, send_json_packet,(void*)ser_socketfd)) != 0)
            {
                perror("send thread create failed.");
                return 1;
            }

            /*�����׽������ݽ����߳�*/
            if((pthread_create(&recv_socket, NULL, recv_json_packet,(void*)ser_socketfd)) != 0)
            {
                perror("recv thread create failed.");
                return 1;
            }
        }
        #if 0
        memset(&re_remote_addr, 0, sizeof(struct sockaddr_in));
        re_namelen = 0;
        char ipAddr[INET_ADDRSTRLEN];
        getsockname(ser_socketfd, (struct sockaddr*)&re_remote_addr, &re_namelen);//��ȡsockfd��ʾ�������ϵı��ص�ַ

        printf("client:client ddress = %s:%d\n", inet_ntop(AF_INET, &re_remote_addr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(re_remote_addr.sin_port));
        //#else

        struct tcp_info info;
        int len=0;
        getsockopt(ser_socketfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
        if(info.tcpi_state == 1)
        {
            printf("is connect!\n");
        }
        else
        {
            printf("disconnect!\n");
            netconnect_flg = 0;
        }
        #endif

    }

	//close(ser_socketfd);
	return 0;

}



/*************************************************
Function:       void send_tick(void*arg)
Description:    ����������
Calls:          makeJson.c(
                        char* create_json_packet(char* code, char* msg, char* serial, cJSON *data)
                        unsigned char *pack_json_packet(char *item)
                        )
Called By:      socket.c(
                      int create_socket()
                    )
Input:          arg-�׽��ֱ�Ƿ�����Ϊ�߳���ں�����������void *���ʹ��Σ�
Output:         ��������������ͨ���׽��ַ���
Return:         void
Others:         NONE
*************************************************/
void send_tick(void*arg)
{
	int fd = (int)arg;
	int len;
	int senderrornum = 0;
	while(1)
	{

        if(netconnect_flg==0)
        {
            return;
        }

        raw_send_buffer_tick = create_json_packet("1100","tick","20160001",NULL);
        len = strlen(raw_send_buffer_tick) + 4;
        send_buffer_tick = pack_json_packet(raw_send_buffer_tick);
        pthread_mutex_lock(&g_mutex[MUTEX_JSON]);/*lock the mutex*/
        if((send(fd,send_buffer_tick,len,0))<0)
        {
            perror("send failed.\n");
            senderrornum++;
            //return;//test wangjj 20160619
        }
        pthread_mutex_unlock(&g_mutex[MUTEX_JSON]);/*unlock the mutex*/


		/****************
		send config data to daemon process
		****************/
		pthread_mutex_lock(&g_mutex[MUTEX_SOCKET]);/*lock the mutex*/
		char heartbuf = 0x11;
		struct msg_communicate msg_sendbuf;
		msg_sendbuf.data = (char *)&heartbuf;
		msg_sendbuf.msgid = SEND_HEART;
		msg_sendbuf.msglen = sizeof(heartbuf);
		msgQsend( msgid[GATEWAY_DAEMON], (char*)(&msg_sendbuf), (msg_sendbuf.msglen+8));

		pthread_mutex_unlock(&g_mutex[MUTEX_SOCKET]);/*unlock the mutex*/
		delay_s(5);//test by wangjj//10
		static int count = 0;
		printf("send heart %d\n", count++);
	}
}


/*************************************************
Function:       void send_json_packet(void *arg)
Description:    ��������JSON���ݰ����ֽ׶ΰ����������а����ļ��ϴ���
Calls:          makeJson.c(
                        unsigned char *pack_json_packet(char *item)
                        )
Called By:      socket.c(
                      int create_socket()
                      )
Input:          arg-�׽��ֱ�Ƿ�����Ϊ�߳���ں�����������void *���ʹ��Σ�
Output:         ��������JSON���ݰ�����ͨ���׽��ַ���
Return:         void
Others:         NONE
*************************************************/
void send_json_packet(void *arg)
{
    int fd = (int)arg;
    int len;
    while(1)
    {
        if(0==netconnect_flg)
        {//eixt and retry to connect
            return;
        }

        if(raw_send_buffer!=NULL)
        {
            len = strlen(raw_send_buffer)+4;
            send_buffer = pack_json_packet(raw_send_buffer);
            pthread_mutex_lock(&g_mutex[MUTEX_JSON]);/*lock the mutex*/
            if((send(fd,send_buffer,len,0))<0)
            {
                perror("send failed.");
                pthread_mutex_unlock(&g_mutex[MUTEX_JSON]);/*lock the mutex*/
                return;
            }
            pthread_mutex_unlock(&g_mutex[MUTEX_JSON]);/*lock the mutex*/
            send_buffer=NULL;
            raw_send_buffer=NULL;
        }
    }
}

/*************************************************
Function:       void recv_json_packet(void *arg)
Description:    ���շ��������͵�JSON���ݰ��������ļ�·���������������
Calls:          resolveJson.c(
                        char* unpack_json_packet(unsigned char* item)
                        )
Called By:      socket.c(
                      int create_socket()
                      )
Input:          arg-�׽��ֱ�Ƿ�����Ϊ�߳���ں�����������void *���ʹ��Σ�
Output:         ���շ��������͵�JSON���ݰ��������ļ�·���������������
Return:         void
Others:         NONE
*************************************************/
void recv_json_packet(void *arg)
{
    int fd = (int)arg;

    int wholelenth; /*��������JSON���ݰ�����*/
    int len;    /*��������JSON���ݰ�����*/

    pthread_t resolve_socket;
    char* recv_buffer;  /*�����ԭ��JSON���ݰ�*/
    char* recv_head;

    /*ѭ�����շ��������͵�����JSON���ݰ�*/
    while(1)
    {
        /*ÿ��������MAX���ֽ����ݣ����ؽ����ֽ���*/
        wholelenth = recv(fd,raw_recv_buffer,MAX,0);
        if(wholelenth<0)
        {
            if(EAGAIN==errno)
            {//server not send data

            }
            else
            {
                perror("recv failed1.\n");
                netconnect_flg = 0;//disconnect , retry connect
                return 1;
            }

        }
        else if(0==wholelenth)
        {
            //printf("recv data = 0\n");
            if(0==errno)
            {

            }
            else
            {
                perror("recv failed2.");
            }
        }
        else
        {
            printf("recv data len = %d\n", wholelenth);
            /*���б��ֲ��*/
            recv_buffer = unpack_json_packet(raw_recv_buffer);
            if(NULL==recv_buffer)
            {
                //printf("recv buffer error,server not update!\n");
                continue;
            }

            len  = strlen(recv_buffer)+4;
            //printf("%d\n",len);

            /*�������ֽ������ݽ����߳�*/
             if((pthread_create(&resolve_socket, NULL, resolve_json_packet,(void*)recv_buffer))!=0)
            {
                perror("recv thread create failed.");
                return 1;
            }

             /*�ȴ��̴߳������*/
            pthread_join(resolve_socket,NULL);

            /*���ֽ������ݰ�����һ��*/
            while(len<wholelenth)
            {
                 /*���*/
                recv_head = &raw_recv_buffer[len];
                recv_buffer = unpack_json_packet(recv_head);
                len += strlen(recv_buffer)+4;

                /*�����������ݽ����߳�*/
                if((pthread_create(&resolve_socket, NULL, resolve_json_packet,(void*)recv_buffer))!=0)
                {
                    perror("recv thread create failed.");
                    return 1;
                }

                /*�ȴ��̴߳������*/
                pthread_join(resolve_socket,NULL);
            }

            /*һ�����ݽ������*/
            printf("1 row recv finished!\n");
            /*�����������ź�ɾ������*/
            play_voice();
        }
        delay_ms(10);
    }
}

/*************************************************
Function:       void resolve_json_packet(void* arg)
Description:    ���շ��������͵�JSON���ݰ��������ļ�·���������������
Calls:          resolveJson.c(
                        int resolve_json(char* json);
                        )
Called By:      socket.c(
                      void recv_json_packet(void *arg)
                      )
Input:          arg-����JSON����Ӧ���ַ�������Ϊ�߳���ں�����������void *���ʹ��Σ�
Output:         ���շ��������͵�JSON���ݰ��������ļ�·���������������
Return:         void
Others:         NONE
*************************************************/
void resolve_json_packet(void* arg)
{
    char* recv_buffer = (char *)arg;
    resolve_json(recv_buffer);
}
