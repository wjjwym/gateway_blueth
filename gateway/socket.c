/*****************************************************************************
Copyright:     2015-2016, FireWork.
File name:     socket.c
Description:   本文件包含与JSON数据包的生成有关的函数，属于工具函数文件
Author:        ShaoYang Yu
Version:       v3.0
Date:          2016-05-15
History:       2016-04-26 v1.0
               2016-05-8  v2.0
               2016-05-11 v3.0
*****************************************************************************/

#include"socket.h"
#include"linux/tcp.h"
/*缓冲区最大字符长度*/
#define MAX 65535

/*服务器IP地址及端口号设置*/
const char* IP = "121.42.196.94";
const int SERVERPORT = 10101;

unsigned char * raw_send_buffer_tick;   /*原生tick数据包预发送缓冲区*/
unsigned char * send_buffer_tick;   /*tick数据包发送缓冲区*/

unsigned char * raw_send_buffer = NULL;    /*原生数据送缓冲区*/
unsigned char raw_recv_buffer[MAX];    /*数据预接收缓冲区*/

unsigned char * send_buffer;    /*数据发送缓冲区*/

static char netconnect_flg = 0;
/*************************************************
Function:       int create_socket()
Description:    初始化socket，并与服务器交互，通过发送或接收JSON数据包
Calls:          socket.c(
                        void send_tick(void*arg)
                        void send_json_packet(void *arg)
                        void recv_json_packet(void *arg)
                        )
Called By:      main.c(
                      int main()
                    )
Input:          NONE
Output:         创建套接字，发送并接收JSON数据包
Return:         0
Others:         NONE
*************************************************/
int create_socket()
{
    int ser_socketfd = -1;	/*创建套接字标记符*/
    struct sockaddr_in remote_addr; 	/*创建socket结构体*/
    pthread_t send_socket_tick; /*创建发送tick线程描述符*/
    pthread_t send_socket;	/*创建发送线程描述符*/
    pthread_t recv_socket;	/*创建接收线程描述符*/
    int serverPort = 0;
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));	/*初始化结构体*/
    remote_addr.sin_family = AF_INET;	/*设置使用网际IPv4协议*/
    remote_addr.sin_addr.s_addr = inet_addr(m_configlist[CONF_SERVERIP].data);	  /*设置IP地址*/
    serverPort = atoi(m_configlist[CONF_SERVERPORT].data);
    remote_addr.sin_port = htons(serverPort);	/*设置服务器端口*/

    struct sockaddr_in re_remote_addr;
    int re_namelen = 0;//getpeername return len

    while(1)/*进入死循环，不执行关闭套接字命令*/
    {
        delay_s(1);
        if(0==netconnect_flg)
        {//retry to connect server
            /*创建套接字*/
            if((ser_socketfd = socket(AF_INET, SOCK_STREAM, 0))<0)
            {
                perror("socket failed.");
                close(ser_socketfd);
                continue;
                //return 1;
            }

            /*将套接字与结构体进行连接，向服务器端发送三次握手信号*/
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

            /*创建套接字tick数据发送线程*/
            if((pthread_create(&send_socket_tick, NULL, send_tick,(void*)ser_socketfd)) != 0)
            {
                perror("send thread create failed.");
                return 1;
            }

            /*创建套接字数据发送线程*/
            if((pthread_create(&send_socket, NULL, send_json_packet,(void*)ser_socketfd)) != 0)
            {
                perror("send thread create failed.");
                return 1;
            }

            /*创建套接字数据接收线程*/
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
        getsockname(ser_socketfd, (struct sockaddr*)&re_remote_addr, &re_namelen);//获取sockfd表示的连接上的本地地址

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
Description:    发送心跳包
Calls:          makeJson.c(
                        char* create_json_packet(char* code, char* msg, char* serial, cJSON *data)
                        unsigned char *pack_json_packet(char *item)
                        )
Called By:      socket.c(
                      int create_socket()
                    )
Input:          arg-套接字标记符（作为线程入口函数，必须以void *类型穿参）
Output:         创建心跳包，并通过套接字发送
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
Description:    发送其他JSON数据包，现阶段包括紧急呼叫包、文件上传包
Calls:          makeJson.c(
                        unsigned char *pack_json_packet(char *item)
                        )
Called By:      socket.c(
                      int create_socket()
                      )
Input:          arg-套接字标记符（作为线程入口函数，必须以void *类型穿参）
Output:         创建其他JSON数据包，并通过套接字发送
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
Description:    接收服务器发送的JSON数据包，包括文件路径包，无命令包等
Calls:          resolveJson.c(
                        char* unpack_json_packet(unsigned char* item)
                        )
Called By:      socket.c(
                      int create_socket()
                      )
Input:          arg-套接字标记符（作为线程入口函数，必须以void *类型穿参）
Output:         接收服务器发送的JSON数据包，包括文件路径包，无命令包等
Return:         void
Others:         NONE
*************************************************/
void recv_json_packet(void *arg)
{
    int fd = (int)arg;

    int wholelenth; /*所有网络JSON数据包长度*/
    int len;    /*单个网络JSON数据包长度*/

    pthread_t resolve_socket;
    char* recv_buffer;  /*拆包后原生JSON数据包*/
    char* recv_head;

    /*循环接收服务器发送的网络JSON数据包*/
    while(1)
    {
        /*每次最多接收MAX个字节数据，返回接收字节数*/
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
            /*进行本轮拆包*/
            recv_buffer = unpack_json_packet(raw_recv_buffer);
            if(NULL==recv_buffer)
            {
                //printf("recv buffer error,server not update!\n");
                continue;
            }

            len  = strlen(recv_buffer)+4;
            //printf("%d\n",len);

            /*创建本轮接收数据解析线程*/
             if((pthread_create(&resolve_socket, NULL, resolve_json_packet,(void*)recv_buffer))!=0)
            {
                perror("recv thread create failed.");
                return 1;
            }

             /*等待线程处理结束*/
            pthread_join(resolve_socket,NULL);

            /*本轮接收数据包多于一个*/
            while(len<wholelenth)
            {
                 /*拆包*/
                recv_head = &raw_recv_buffer[len];
                recv_buffer = unpack_json_packet(recv_head);
                len += strlen(recv_buffer)+4;

                /*创建接收数据解析线程*/
                if((pthread_create(&resolve_socket, NULL, resolve_json_packet,(void*)recv_buffer))!=0)
                {
                    perror("recv thread create failed.");
                    return 1;
                }

                /*等待线程处理结束*/
                pthread_join(resolve_socket,NULL);
            }

            /*一轮数据接收完成*/
            printf("1 row recv finished!\n");
            /*进行语音播放和删除操作*/
            play_voice();
        }
        delay_ms(10);
    }
}

/*************************************************
Function:       void resolve_json_packet(void* arg)
Description:    接收服务器发送的JSON数据包，包括文件路径包，无命令包等
Calls:          resolveJson.c(
                        int resolve_json(char* json);
                        )
Called By:      socket.c(
                      void recv_json_packet(void *arg)
                      )
Input:          arg-接收JSON包对应的字符串（作为线程入口函数，必须以void *类型穿参）
Output:         接收服务器发送的JSON数据包，包括文件路径包，无命令包等
Return:         void
Others:         NONE
*************************************************/
void resolve_json_packet(void* arg)
{
    char* recv_buffer = (char *)arg;
    resolve_json(recv_buffer);
}
