#ifndef DESENCRYPTION_H_INCLUDED
#define DESENCRYPTION_H_INCLUDED

//====================研５专用接口=======================
#include "stdlib.h"

#define g_ENCRYPT_FLAG 1
#define g_DECRYPT_FLAG 2
#define g_YAN5_DES_LEN 128

typedef struct g_Yan5DES
{//研五加解密专用结构
    unsigned char indata[g_YAN5_DES_LEN];
    unsigned char outdata[g_YAN5_DES_LEN];
    unsigned char key[8];
    unsigned long len;
} gYan5DES;
unsigned char enY5DES(gYan5DES* y5);//加密
unsigned char unY5DES(gYan5DES* y5);//解密
void clrYan5DES(gYan5DES* y5);
//====================CBC-DES和ECB-DES接口=====================
unsigned char enDES_cbc(unsigned char* indata,unsigned long inlen,unsigned char key[8],\
        unsigned char* outdata,unsigned long* outlen,unsigned char* iv);//加密
unsigned char unDES_cbc(unsigned char* indata,unsigned long inlen,unsigned char key[8],\
        unsigned char* outdata,unsigned long* outlen,unsigned char* iv);//解密
unsigned char enDES(unsigned char* indata,unsigned long inlen,unsigned char key[8],\
        unsigned char* outdata,unsigned long* outlen);//加密
unsigned char unDES(unsigned char* indata,unsigned long inlen,unsigned char key[8],\
        unsigned char* outdata,unsigned long* outlen);//解密
void randKey(unsigned char key[8]);//生成随机密钥
//====================以下函数自己调用========================
//加解密的公共函数
unsigned char comDES(unsigned char in[8],unsigned char out[8],\
        unsigned char subkey[16][8],unsigned char flg);
//生成16个子密钥(48位有效)
void genKey(unsigned char*,unsigned char[16][8]);
//S盒代替和P置换(48位->32位)
void sReplace(unsigned char*) ;

#endif // DESENCRYPTION_H_INCLUDED
