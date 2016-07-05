#include "encryption.h"
//same as decryption
unsigned char DES_key[8] =
{
    8,8,7,7,3,3,6,5
};


/*************************************************
Function:       char* encryption_data(const char *p_conf_file, int filelen, int *convertlen)
Description:    加密数据
Calls:          desEncryption.c(
                      void genKey(unsigned char *key,unsigned char nkey[16][8]);
                      unsigned char comDES(unsigned char in[8],unsigned char out[8],
        unsigned char subkey[16][8],unsigned char flg);
                      )
Called By:      encryption.c(
                      void process_configfile()
                      )
Input:          const char *p_conf_file：配置文件指针
                int filelen： 文件长度
                int *convertlen转换后到文件长度
Output:         int *convertlen转换后到文件长度
                char *convertbuf：转换后到文件缓存指针
Return:         char *convertbuf：转换后到文件缓存指针
Others:         NONE
*************************************************/
char* encryption_data(const char *p_conf_file, int filelen, int *convertlen)
{
    char *expendbuf = NULL;
    char *convertbuf = NULL;
    unsigned char m_key[16][8];
    memset(m_key ,0, sizeof(m_key));
    int convert_index=0;

    /*扩展到8个字节*/
    *convertlen = filelen + filelen%8;
    expendbuf = malloc(*convertlen);
    convertbuf = malloc(*convertlen);
    memset(expendbuf, 0x0, *convertlen);
    memset(convertbuf, 0x0, *convertlen);
    memcpy(expendbuf, p_conf_file, filelen);

    /*获取加密密匙*/
    genKey(DES_key, m_key);

     /*加密数据*/
    for(convert_index = 0; convert_index < *convertlen; convert_index+=8)
    {
        unsigned char in[8] = {0};
        unsigned char out[8] = {0};
        memcpy(in, &(expendbuf[convert_index]), sizeof(in));
        comDES(in, out, m_key, g_ENCRYPT_FLAG);
        memcpy(&convertbuf[convert_index], out,sizeof(out));
    }

    free(expendbuf);
    return convertbuf;//返回加密后的数据
}

/*************************************************
Function:       void process_configfile(const char *p_conf_file, int filelen)
Description:    处理配置文件
Calls:          encryption.c(
                      char* encryption_data(const char *p_conf_file, int filelen, int *convertlen);
                      void save_configfile(char* convertbuf, int filelen);
                      )
Called By:      msgprocess.c(
                      void msgparse(struct msg_communicate *recvdata)
                      )
Input:          const char *p_conf_file：配置文件指针
                int filelen： 文件长度
Output:         int *convertlen转换后到文件长度
                char *convertbuf：转换后到文件缓存指针
Return:         NONE
Others:         NONE
*************************************************/
void process_configfile(const char *p_conf_file, int filelen)
{
    char *convertbuf = NULL;
    int convertlen = 0;
    /*加密数据*/
    convertbuf = encryption_data(p_conf_file, filelen, &convertlen);
    /*保存加密后到配置文件到本地*/
    save_configfile(convertbuf, convertlen);
}


/*************************************************
Function:       void save_configfile(char* convertbuf, int filelen)
Description:    保存加密后到配置文件到本地
Calls:          NONE
Called By:      msgprocess.c(
                      void process_configfile(const char *p_conf_file, int filelen)
                      )
Input:          char *convertbuf：转换后到文件缓存指针
                int filelen： 文件长度
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
void save_configfile(char* convertbuf, int filelen)
{
    int fd_conf = -1;
    //打开本地到配置文件并且重写覆盖
    fd_conf = open(PATH_CONFIGFILE, O_RDWR|O_CREAT, 0777);
    if(fd_conf < 0)
    {
        perror("open config file error!\n");
        return;
    }
    //写入加密后的配置文件
    if(filelen!=write(fd_conf, convertbuf, filelen))
    {
        perror("write to gateway_config.bin error\n");
    }
    else
    {
        printf("suc to save config file\n");
    }
    close(fd_conf);

}
