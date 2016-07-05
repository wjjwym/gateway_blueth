#include "bluetoothpro.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int main(int argc, char **argv)
{
    struct sockaddr_l2 addr = { 0 };
    int s, status;
    char *message = "hello!";
    char dest[18] = "5C:AD:CF:2C:90:F3";

    if(argc < 2)
    {
        fprintf(stderr, "usage: %s <bt_addr>\n", argv[0]);
        exit(2);
    }

    strncpy(dest, argv[1], 18);

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x1001);
    str2ba( dest, &addr.l2_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( status == 0 ) {
        status = write(s, "hello!", 6);
    }

    if( status < 0 ) perror("uh oh");

    close(s);
}

#endif
#if 1

void bluetooth_init()
{

    pthread_t th_bluet;

    if((pthread_create(&th_bluet, NULL, bluet_connect, NULL)) != 0)
    {
        perror("bluet_connect thread create failed.");
        return 1;
    }
    pthread_join(th_bluet, NULL);

    //return 0;
}


void bluet_connect()
{

    while(1)
    {
        inquiry_info *ii = NULL;
        int max_rsp, num_rsp;
        int dev_id, sock, len, flags;
        int i;
        char addr[19] = { 0 };
        char name[248] = { 0 };
        dev_id = hci_get_route(NULL);//返回dongle Device ID
        sock = hci_open_dev(dev_id);//HCI命令发送前都需要打开并绑定
        if (dev_id < 0 || sock < 0) {
            perror("opening socket");
            exit(1);
        }

        len  = 8;
        max_rsp = 255;
        flags = IREQ_CACHE_FLUSH;
        ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

        num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);//搜索周围所有的蓝牙设备，并将搜索到的bdaddr传递回来
        printf("scan device num = %d\n", num_rsp);
        if( num_rsp < 0 )
        {
            perror("hci_inquiry");
        }

        for (i = 0; i < num_rsp; i++) {
            ba2str(&(ii+i)->bdaddr, addr);//将bdaddr转换为字符串
            memset(name, 0, sizeof(name));
            if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name),
                name, 0) < 0)
            strcpy(name, "[unknown]");
            printf("%s  %s\n", addr, name);

            bluetooth_socket(&(ii+i)->bdaddr);
        }

        free( ii );
        close( sock );
        usleep(100*1000);
    }
}
#endif
#if 0
void bluetooth_init()
{

    //hci info get
    struct hci_dev_list_req *dl;
    struct hci_dev_req *dr;
    struct hci_dev_info di;

    if (!(dl = malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t))))
    {
        perror("Can't allocate memory");
        exit(1);
    }
    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    //create a socket
    int bt_socket;
    bt_socket = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
    //domain=PF_BLUETOOTH, type可以是多种类型。protocol=BTPROTO_L2CAP;

    // 2. 使用HCIGETDEVLIST,得到所有device的Device ID。存放在dl中。
    if (ioctl(bt_socket, HCIGETDEVLIST, (void *) dl)< 0)
    {
        perror("Can't get device list");
        exit(1);
    }
    //2.绑定：
    // Bind to local address
    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, &di.bdaddr); //bdaddr为本地Dongle BDAddr
    if (bind(bt_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("Can't bind socket");
        return;
    }


    //3.连接
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    //bacpy(addr.l2_bdaddr, src);
    /*L2CAP可以复用发给上层协议的连接请求，这些上层协议包括服务发现协议SDP（PSM = 0x0001）、
      RFCOMM（PSM = 0x0003）和电话控制（PSM = 0x0005）等。*/
    addr.l2_psm = 0x0001;//SDP
    if (connect(bt_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("Can't connect");
        return;
    }

}
#endif
//bluetooth client
void bluetooth_socket(bdaddr_t *ba)
{
    static socketconnct = 0;
    if(1 == socketconnct)
    {
        return ;
    }
    //create a socket
    int bt_socket = 0;
    bt_socket = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
    if(bt_socket<0)
    {
        perror("create bt socket error!\n");
    }
    //domain=PF_BLUETOOTH, type可以是多种类型。protocol=BTPROTO_L2CAP;

    //连接
    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));

    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, ba); //bdaddr为remote device Dongle BDAddr

    /*L2CAP可以复用发给上层协议的连接请求，这些上层协议包括服务发现协议SDP（PSM = 0x0001）、
      RFCOMM（PSM = 0x0003）和电话控制（PSM = 0x0005）等。*/
    addr.l2_psm = 0x0001;//SDP
    if (connect(bt_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("Can't connect");
        socketconnct = 0;
        return;
    }
    else
    {
        socketconnct = 1;
    }

    /*创建套接字数据接收线程*/
    if((pthread_create(&bt_socket, NULL, bluet_recv,(void*)bt_socket)) != 0)
    {
        perror("recv thread create failed.");
        return 1;
    }
    pthread_join(bt_socket, NULL);



}
/*
注意：
struct sockaddr_l2 {
 sa_family_t l2_family;  //必须为 AF_BLUETOOTH
 unsigned short l2_psm;  //与前面PSM对应,这一项很重要
 bdaddr_t l2_bdaddr;     //Remote Device BDADDR
 unsigned short l2_cid;
};

4. 发送数据到Remote Device：
send()或write()都可以。

5. 接收数据：
revc() 或read()

*/


#define BLUET_MAXBUF 1024
char bt_recv_buffer[BLUET_MAXBUF];
void bluet_recv(void *arg)
{
    int fd = (int)arg;

    int wholelenth; /*所有网络JSON数据包长度*/
    int len;    /*单个网络JSON数据包长度*/

    pthread_t resolve_socket;
    char* recv_buffer;  /*拆包后原生JSON数据包*/
    char* recv_head;
    fd_set set;
    unsigned long iocflag = 1;

    //ioctl(fd ,FIONBIO, &iocflag);
    /*循环接收服务器发送的网络JSON数据包*/
    while(1)
    {
        delay_ms(10);
        /*每次最多接收MAX个字节数据，返回接收字节数*/
        //FD_ZERO(&set);
        //FD_SET(fd, &set);
        //select(0, &set, NULL, NULL, NULL);
        //if(FD_ISSET(fd, &set))
        {

            wholelenth = recv(fd,bt_recv_buffer,BLUET_MAXBUF,0);
            if(wholelenth<0)
            {
                perror("recv socket data len error\n");
                //return;
            }
            else if(0==wholelenth)
            {
                perror("recv socket data len = 0\n");
            }
            else
            {
                int i = 0;

                printf("wholelenth = %d \n", wholelenth);
                for(i = 0; i < wholelenth; i++)
                {
                    printf("=%x=\n", bt_recv_buffer[i]);
                }
            }
        }
        //else
        {
            //perror("not recv data\n");
        }
    }
}



void hcidevinfo_get()
{

}

