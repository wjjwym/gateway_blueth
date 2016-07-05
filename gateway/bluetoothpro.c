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
        dev_id = hci_get_route(NULL);//����dongle Device ID
        sock = hci_open_dev(dev_id);//HCI�����ǰ����Ҫ�򿪲���
        if (dev_id < 0 || sock < 0) {
            perror("opening socket");
            exit(1);
        }

        len  = 8;
        max_rsp = 255;
        flags = IREQ_CACHE_FLUSH;
        ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

        num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);//������Χ���е������豸��������������bdaddr���ݻ���
        printf("scan device num = %d\n", num_rsp);
        if( num_rsp < 0 )
        {
            perror("hci_inquiry");
        }

        for (i = 0; i < num_rsp; i++) {
            ba2str(&(ii+i)->bdaddr, addr);//��bdaddrת��Ϊ�ַ���
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
    //domain=PF_BLUETOOTH, type�����Ƕ������͡�protocol=BTPROTO_L2CAP;

    // 2. ʹ��HCIGETDEVLIST,�õ�����device��Device ID�������dl�С�
    if (ioctl(bt_socket, HCIGETDEVLIST, (void *) dl)< 0)
    {
        perror("Can't get device list");
        exit(1);
    }
    //2.�󶨣�
    // Bind to local address
    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, &di.bdaddr); //bdaddrΪ����Dongle BDAddr
    if (bind(bt_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("Can't bind socket");
        return;
    }


    //3.����
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    //bacpy(addr.l2_bdaddr, src);
    /*L2CAP���Ը��÷����ϲ�Э�������������Щ�ϲ�Э�����������Э��SDP��PSM = 0x0001����
      RFCOMM��PSM = 0x0003���͵绰���ƣ�PSM = 0x0005���ȡ�*/
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
    //domain=PF_BLUETOOTH, type�����Ƕ������͡�protocol=BTPROTO_L2CAP;

    //����
    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));

    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, ba); //bdaddrΪremote device Dongle BDAddr

    /*L2CAP���Ը��÷����ϲ�Э�������������Щ�ϲ�Э�����������Э��SDP��PSM = 0x0001����
      RFCOMM��PSM = 0x0003���͵绰���ƣ�PSM = 0x0005���ȡ�*/
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

    /*�����׽������ݽ����߳�*/
    if((pthread_create(&bt_socket, NULL, bluet_recv,(void*)bt_socket)) != 0)
    {
        perror("recv thread create failed.");
        return 1;
    }
    pthread_join(bt_socket, NULL);



}
/*
ע�⣺
struct sockaddr_l2 {
 sa_family_t l2_family;  //����Ϊ AF_BLUETOOTH
 unsigned short l2_psm;  //��ǰ��PSM��Ӧ,��һ�����Ҫ
 bdaddr_t l2_bdaddr;     //Remote Device BDADDR
 unsigned short l2_cid;
};

4. �������ݵ�Remote Device��
send()��write()�����ԡ�

5. �������ݣ�
revc() ��read()

*/


#define BLUET_MAXBUF 1024
char bt_recv_buffer[BLUET_MAXBUF];
void bluet_recv(void *arg)
{
    int fd = (int)arg;

    int wholelenth; /*��������JSON���ݰ�����*/
    int len;    /*��������JSON���ݰ�����*/

    pthread_t resolve_socket;
    char* recv_buffer;  /*�����ԭ��JSON���ݰ�*/
    char* recv_head;
    fd_set set;
    unsigned long iocflag = 1;

    //ioctl(fd ,FIONBIO, &iocflag);
    /*ѭ�����շ��������͵�����JSON���ݰ�*/
    while(1)
    {
        delay_ms(10);
        /*ÿ��������MAX���ֽ����ݣ����ؽ����ֽ���*/
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

