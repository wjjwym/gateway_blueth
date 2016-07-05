/*****************************************************************************
Copyright:     2015-2016, Firework.
File name:     sysinit.h
Description:   此文件主要完成家庭网关系统按键初始化的相关操作，使用了树莓派中
               的wiringPi库作为GPIO控制库
Author:        YU SHAO YANG
Version:       1.0
Date:          2016.04.02
History:       NONE
*****************************************************************************/
#include"sysinit.h"
/*************************************************
Function:       gateway_init
Description:    该函数用于执行四个按键的初始化和中断
                设置工作。
Calls:          wiringPi.h(
                            int wiringPiSetup(void);
                            void pinMode(int pin, int mode);
                            void pullUpDnControl(int pin, int pud);
                            int wiringPiISR(int pin, int edgeType,
                                       void(*function)(void));
                          )
Called By:      main.c
Input:          NONE
Output:         void KeyInterrupt1(void);
                void KeyInterrupt2(void);
                void KeyInterrupt3(void);
                void KeyInterrupt4(void);
Return:         NONE
Others:         NONE
*************************************************/
void gateway_init()
{
    init_globalvalue();
    /*create process communicate between*/
    init_msgprocess();
    /*send init quest to daemon, wait for answer*/
    //handshake_withdaemon();

    init_configfile();

    //bluetooth initialize
    //bluetooth_init();

    //listen the key down
    init_listen_key("/dev/input/event2");

    /*wiringPi库初始化函数，需要ROOT权限执行*/
    wiringPiSetup();

    /*将四个按键模式均设置为输入捕获模式*/
	pinMode(Key1,INPUT);
	pinMode(Key2,INPUT);
	pinMode(Key3,INPUT);
	pinMode(Key4,INPUT);

	/*将四个按键所对应的端口设置为有上拉电阻模式，内部电压上拉至3.3V*/
	pullUpDnControl(Key1, PUD_UP);
	pullUpDnControl(Key2, PUD_UP);
	pullUpDnControl(Key3, PUD_UP);
	pullUpDnControl(Key4, PUD_UP);

	/*设置四个按键为下降沿触发中断，并设置中断服务程序名*/
	wiringPiISR(Key1,INT_EDGE_FALLING,&key_interrupt_1);
	wiringPiISR(Key2,INT_EDGE_FALLING,&key_interrupt_2);
	wiringPiISR(Key3,INT_EDGE_FALLING,&key_interrupt_3);
	wiringPiISR(Key4,INT_EDGE_FALLING,&key_interrupt_4);
}


/*************************************************
Function:       delay_us
Description:    该函数用于微秒延时。
Calls:          unistd.h(
                            void usleep(int micro_seconds);
                        )
Called By:      main.c
                interrupt.c
                //
Input:          unsigned int useconds
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
void delay_us(unsigned int useconds)
{
    usleep(useconds);
}

/*************************************************
Function:       delay_ms
Description:    该函数用于毫秒延时。
Calls:          unistd.h(
                            void usleep(int micro_seconds);
                        )
Called By:      main.c
                interrupt.c
                //
Input:          unsigned int mseconds
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
void delay_ms(unsigned int mseconds)
{
    usleep(mseconds*1000);
}

/*************************************************
Function:       delay_s
Description:    该函数用于毫秒延时。
Calls:          unistd.h(
                             unsigned sleep(unsigned int seconds);
                        )
Called By:      main.c
                interrupt.c
                //
Input:          unsigned int seconds
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
void delay_s(unsigned int seconds)
{
    usleep(seconds*1000*1000);
}

/*************************************************
Function:       init_configfile
Description:    ?ú????????????????????
Calls:          unistd.h(
                             char* get_file(const char *filename, int filelen);
                             unsigned int get_length(const char *filename);
                             char*  decryption_data(const char *p_conf_file, int filelen, int *convertlen);
                        )
Called By:     void gateway_init()
Input:          NONE
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
void init_configfile()
{
    char* p_conf_file = NULL;
    char* convertbuf = NULL;
    int len_configfile = 0, convertlen = 0;
    struct msg_communicate msg_sendbuf;
    memset(&msg_sendbuf, 0x0, sizeof(msg_sendbuf));

    if(1==ENCRYPTION)
    {//the file with encryption type "bin"
        len_configfile = get_length(CONF_FILENAME_ENCRP);
        p_conf_file = get_file(CONF_FILENAME_ENCRP,len_configfile);
        if( NULL==p_conf_file )
        {
            perror("init gateway_config.bin file failed!\n");
        }

        //encryption data

        convertbuf = decryption_data(p_conf_file, len_configfile, &convertlen);

        msg_sendbuf.data = convertbuf;
        msg_sendbuf.msglen = convertlen;
    }
    else
    {//read the file type "txt" without encryption
        len_configfile = get_length(CONF_FILENAME);
        p_conf_file = get_file(CONF_FILENAME,len_configfile);
        if( NULL==p_conf_file )
        {
            perror("init gateway_config.txt file failed!\n");
        }
        msg_sendbuf.data = p_conf_file;
        msg_sendbuf.msglen = len_configfile;

    }

    pthread_mutex_lock(&g_mutex[MUTEX_SOCKET]);/*lock the mutex*/

    //send config data to daemon process
    msg_sendbuf.msgid = SEND_CONFFILE;

    msgQsend( msgid[GATEWAY_DAEMON], (char*)(&msg_sendbuf), (msg_sendbuf.msglen+8));

    pthread_mutex_unlock(&g_mutex[MUTEX_SOCKET]);/*unlock the mutex*/

    parse_config(p_conf_file);
    free(p_conf_file);
}


/*************************************************
Function:       char* get_file(char *bfile)
Description:
Calls:          NONE
Called By:      sysinit.c(
                         void init_configfile()
                        )
Table Accessed: NONE
Table Updated:  NONE
Input:          filename
Output:         file content
Return:         char* p_file_content
Others:         NONE
*************************************************/
char* get_file(const char *filename, int filelen)
{
    int conf_fd = 0;
    char *p_conf_file;

    /*ues file len to alloc*/
    if( 0!=filelen )
    {
        p_conf_file = malloc(filelen);
    }
    else
    {
        perror("get config file len error!\n");
        return NULL;
    }

    /*get config file content*/
    conf_fd = open(filename, O_RDWR, 0777);
    if(conf_fd < 0)
    {
        perror("exist config file!\n");
        return NULL;
    }
    else
    {
        read(conf_fd, p_conf_file, filelen);
        return p_conf_file;
    }
}

/*************************************************
Function:       int get_length(char *bfile)
Description:
Calls:          NONE
Called By:      sysinit.c(
                         void init_configfile()
                        )
Table Accessed: NONE
Table Updated:  NONE
Input:          filename
Output:         filesize
Return:         filesize
Others:         NONE
*************************************************/
unsigned int get_length(const char *filename)
{

    /*fd?????????????SIZE:??????????????????????????*/
    int fd = 0, SIZE = 0;

    if((fd = open(filename, O_RDWR, 0777)) < 0){
        perror("File open error");
        return 0;
    }
    SIZE = lseek(fd, 0, SEEK_END);
    close(fd);

    return SIZE;
}

char conf_list[MAX_CONFIG];
/*************************************************
Function:       void parse_config(char *p_conf_file)
Description:
Calls:          NONE
Called By:      sysinit.c(
                         void init_configfile()
                        )
Table Accessed: NONE
Table Updated:  NONE
Input:          p_conf_file
Output:         filesize
Return:         filesize
Others:         NONE
*************************************************/
void parse_config(char *p_conf_file)
{
    int configlist_index=0;
    if( NULL!=p_conf_file )
    {

        memset(m_configlist, 0x0, sizeof(m_configlist));
        /*get Server IP addrs*/
        m_configlist[configlist_index] = find_string(p_conf_file, "ServerIP=");
        if(NULL==m_configlist[configlist_index].data)
        {//set default value
            memcpy(m_configlist[configlist_index].data, "121.42.196.94",sizeof("121.42.196.94"));
        }
        configlist_index++;

        /*get Server Port Number*/
        m_configlist[configlist_index] = find_string(p_conf_file, "ServerPort=");
        if(NULL==m_configlist[configlist_index].data)
        {//set default value
            memcpy(m_configlist[configlist_index].data, "10101",sizeof("10101"));
        }
        configlist_index++;

        /*get interruptDelay Value*/
        m_configlist[configlist_index] = find_string(p_conf_file, "InterruptDelay=");
        if(NULL==m_configlist[configlist_index].data)
        {//set default value
            memcpy(m_configlist[configlist_index].data, "150",sizeof("150"));
        }
        configlist_index++;

        /*get DownloadAddr Value*/
        m_configlist[configlist_index] = find_string(p_conf_file, "DownloadAddr=");
        if(NULL==m_configlist[configlist_index].data)
        {//set default value
            memcpy(m_configlist[configlist_index].data, "http://121.42.196.94:8080/",sizeof("http://121.42.196.94:8080/"));
        }
        configlist_index++;
    }
}


struct _CONFIGSTRING_ find_string(const char *srcstr, const char *deststr)
{
    char *find_str = NULL;
    int dataindex = 0;
    struct _CONFIGSTRING_ m_configstr;
    memset( &m_configstr, 0, sizeof(m_configstr) );
    find_str = strstr( srcstr, deststr );
    if( NULL!=find_str )
    {
        find_str += strlen(deststr);
        dataindex = 0;
        /*get dest string*/
        while( ( '#'!=*find_str ) && ( '\0'!=*find_str ) )
        {//every time copy one byte
            memcpy( &(m_configstr.data[dataindex]), find_str,1 );
            dataindex++;
            m_configstr.len++;
            find_str++;
        }
    }
    else
    {
        printf("find %s error!\n",deststr);
    }
    return m_configstr;
}

//same as encryption
unsigned char DES_key[8]=
{
    8,8,7,7,3,3,6,5
};

char*  decryption_data(const char *p_conf_file, int filelen, int *convertlen)
{
    char *expendbuf = NULL;
    char *convertbuf = NULL;
    unsigned char m_key[16][8];
    memset(m_key ,0, sizeof(m_key));
    int convert_index=0;

    /*expend to 8 bytes*/
    *convertlen = filelen + filelen%8;
    expendbuf = malloc(*convertlen);
    convertbuf = malloc(*convertlen);
    memset(expendbuf, 0x0, *convertlen);
    memset(convertbuf, 0x0, *convertlen);
    memcpy(expendbuf, p_conf_file, filelen);

    /*get des encryption key*/
    genKey(DES_key, m_key);

     /*encryption the data*/
    for(convert_index = 0; convert_index < *convertlen; convert_index+=8)
    {
        unsigned char in[8] = {0};
        unsigned char out[8] = {0};
        memcpy(in, &(expendbuf[convert_index]), sizeof(in));
        comDES(in, out, m_key, g_DECRYPT_FLAG);
        memcpy(&(convertbuf[convert_index]), out, sizeof(out));
    }
    free(expendbuf);
    return convertbuf;
}

void init_msgprocess()
{
    pthread_t msgpro;  /*?????????????????*/
    /*????????process???*/
    if(pthread_create(&msgpro, NULL, msgprocess,NULL) != 0)
    {
        perror("msgprocess create failed.");
        exit(1);
    }
}



void init_listen_key(char *dev_event)
{
    pthread_t thd_key;
    if(pthread_create(&thd_key, NULL, listen_key, dev_event)!=0)
    {
        perror("listen_key thread create failed.");
        return 1;
    }
}



void init_globalvalue()
{
    int mutex_index = 0 ;
    for(mutex_index = 0; mutex_index < MUTEX_MAX; mutex_index++)
    {
        if(pthread_mutex_init(&g_mutex[mutex_index],NULL))
        {
            printf("init mutex index = %d error!\n", mutex_index);
        }
    }
}
