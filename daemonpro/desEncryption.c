//作者：高金山
//日期：2004年3月
//说明：本文件包含了DES加密解密的函数。
//循环移位表,PC-1表,PC-2表,IP表,IP-1表,
//E扩展表,P变换表,S盒等都可以自己设置

#include "desEncryption.h"

#define bmalloc malloc
void xTran(unsigned char q[8],unsigned char p[8],unsigned char xTab[],int xLen)
{//根据置换表xTab将64位q转换成p
    int i,qt,pt,tt;//qt,pt分别表示q,p的第几个字节,tt暂存
    for(i=0;i<8;i++)
        p[i]=0;//先清零
    for(i=0;i<xLen;i++)
    {//循环置位
        pt=i/8;
        qt=(xTab[i]-1)/8;
        tt=q[qt] << ((xTab[i]-1) % 8);
        tt=tt & 0x80;//1000 0000
        tt=tt >> (i % 8);
        p[pt]=p[pt] | tt;
    }
}
void genKey(unsigned char *key,unsigned char nkey[16][8])
{//由初始密钥okey生成16个子密钥nkey
    unsigned char movebit[16]={//循环移位表。
        1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    unsigned char pc_1[56]={//PC-1置换。
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4};
    unsigned char pc_2[48]={//PC-2置换。
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32};
    unsigned char tkey[8],tt[8];//暂存64字节
    unsigned long key_c,key_d; //前28位,后28位
    int i=0,j=0;
    for(i=0;i<8;i++) tkey[i]=0;
    i=0;//strcpy(tkey,key);
    while((*(key+i)!='\0')&&(i<8))
    {    tkey[i]=*(key+i);i++; }
    xTran(tkey,tt,pc_1,56); //PC-1置换
    key_c=(*(tt+0)<<24)+(*(tt+1)<<16)+(*(tt+2)<<8)+(*(tt+3));
    key_c=key_c & 0xfffffff0;//将前28位移入key_c的高28位
    key_d=(*(tt+3)<<24)+(*(tt+4)<<16)+(*(tt+5)<<8)+(*(tt+6));
    key_d=key_d & 0x0fffffff;//将后28位移入key_d低28位
    for(i=0;i<16;i++)
    { //移位以得到各个子密钥
        //根据移位表对c,d进行循环左移
        key_c=(key_c<<movebit[i]) | ((key_c>>(28-movebit[i])) & 0xfffffff0);
        key_d=((key_d<<movebit[i])& 0x0fffffff) | (key_d>>(28-movebit[i]));
        for(j=0;j<8;j++)
            tt[j]=0;//清零
        *(tt+0)=key_c>>24;*(tt+1)=key_c>>16;
        *(tt+2)=key_c>>8;*(tt+3)=key_c;//合并c到tt
        *(tt+3)|=(key_d>>24);*(tt+4)=key_d>>16;
        *(tt+5)=key_d>>8;*(tt+6)=key_d;//合并d到tt
        xTran(tt,nkey[i],pc_2,48);//PC-2置换
    }
}//生产子密钥结束
void sReplace(unsigned char right_s[8])
{//查找S盒，把扩展成48位的数据，替换成32位的数据
    unsigned char p[32]={//P置换。
        16,7,20,21,
        29,12,28,17,
        1,15,23,26,
        5,18,31,10,
        2,8,24,14,
        32,27,3,9,
        19,13,30,6,
        22,11,4,25 } ;
    unsigned char s[][4][16] =
        {{//S盒1。   S盒都可以自己设置
        14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
        0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
        4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
        15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13 },
        {//S盒2
        15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
        3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
        0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
        13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9 },
        {//S盒3
        10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
        13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
        13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
        1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 },
        {//S盒4
    7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
        13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
        10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
        3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14 },
        { //S盒5
        2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
        14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
        4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
        11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3 },
        {//S盒6
        12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
        10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
        9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
        4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 },
        {//S盒7
        4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
        13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
        1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
        6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12 },
        {//S盒8
        13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
        1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
        7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
        2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11 }};
    unsigned char row,col,tmp_s[8]={0,0,0,0,0,0,0,0};
    unsigned short tt;
    int i=0;
    for(i=0;i<=7;i++)
{//从S盒中取数
        row=0,col=0;tt=0;
        tt=((right_s[i*6/8]<<8) + (right_s[i*6/8+1])) >> (10-i*6 % 8);//取六位放到tt低6位
        row=(row | ((tt>>5) & 0x01)) << 1;//取6位的第0位付给row的6位
        row=(row | ((tt>>0) & 0x01)) << 0;//取6位的第5位付给row的7位
        col=(tt >> 1) & 0x0f;//取6位的第1,2,3,4位付给row的低四位
     //根据行和列的值从S盒中取数
     tmp_s[i/2]=tmp_s[i/2]|s[i][row][col]<<4*((i+1)%2);
} //s_out[0-3]:???????? ???????? ???????? ???????? 00000000...
    xTran(tmp_s,right_s,p,32); //P置换
}//S代替的结束

unsigned char comDES(unsigned char in[8],unsigned char out[8],
        unsigned char subkey[16][8],unsigned char flg)
{//单重DES加解密的公共函数 in输入数据 key密钥   out输出数据
    unsigned char ip[64]={////IP置换。
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9, 1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7 };
    unsigned char ip_1[64]={//IP-1置换。可根据IP表生成
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9, 49,17,57,25 };
    unsigned char e[48] ={//E扩展。
        32,1, 2, 3, 4, 5,
        4, 5, 6, 7, 8, 9,
        8, 9, 10,11,12,13,
        12,13,14,15,16,17,
        16,17,18,19,20,21,
        20,21,22,23,24,25,
        24,25,26,27,28,29,
        28,29,30,31,32,1};
    unsigned char left[8],right[8],temp[8];//数据的左32位,右32位,暂存
    int i=0,j=0,k=0;
    xTran(in,temp,ip,64); //IP置换
    for(i=0;i<=3;i++)   left[i]=temp[i] ;//将明文左32位放到left中
    for(i=4;i<=7;i++)   right[i-4]=temp[i] ;//将明文右32位放到right中
    for(i=0;i<16;i++)
{//16层循环
        if(flg==g_ENCRYPT_FLAG)         k=i;
        else if(flg==g_DECRYPT_FLAG)    k=15-i;
        else return 0;
     for(j=0;j<=3;j++)
        {
            temp[j]=left[j];//将left暂存起来
            left[j]=right[j];//L(n) = R(n-1)
        }
        xTran(left,right,e,48);//对right进行E扩展
     for(j=0;j<6;j++)
            right[j]=right[j]^subkey[k][j];//48位的数据右部与密钥异或
     sReplace(right) ;//将48位的right变为32位
     for(j=0;j<=3;j++)
   {//获得下一个right
      right[j]=temp[j] ^ right[j] ;//f(R(n-1),k)
   }
}
    for(i=0;i<4;i++)   temp[i]=right[i]; //合并right和left到temp(64位)
    for(i=4;i<8;i++)   temp[i]=left[i-4];//右边左边互换,因为最后一次不用交换
    //for(i=0;i<64;i++) ip_1[ip[i]-1]=i+1;//生成IP-1置换表
    xTran(temp,out,ip_1,64);//IP-1置换
    return 1;
}//单重DES加解密结束
//=======================================DES====================================
unsigned char enDES(unsigned char* indata,unsigned long inlen,unsigned char* key,\
        unsigned char* outdata,unsigned long* outlen)
{//加密
    unsigned char *p,*p1,s_key[16][8],tt[8];
    long tlen=0,i=0,ttlen=0;
    if((indata==NULL)||(outdata==NULL)||(key==NULL)
        ||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
        return 0;//传入参数错误
    p=indata;
    tlen=inlen/8;
    if(inlen%8!=0)
        tlen=tlen+1;
    tlen=tlen*8;//补整
    *outlen=tlen;//密文长度
    p1=outdata;tlen=inlen;
    for(i=0;i<8;i++)    tt[i]=0;
    genKey(key,s_key);//获取16个子密钥
    while(tlen>0)
    {//逐64位循环加密
        for(i=0;i<8;i++) tt[i]=0;
        ttlen=(tlen<8)?tlen:8;
        for(i=0;i<ttlen;i++)
            tt[i]=*(p+i);//取原数据
        comDES(tt,p1,s_key,g_ENCRYPT_FLAG);
        p=p+8;p1=p1+8;tlen=tlen-8;
    }
    return 1;
}
unsigned char unDES(unsigned char* indata,unsigned long inlen,unsigned char* key,\
        unsigned char* outdata,unsigned long* outlen)
{//解密
    unsigned char *p,*p1,s_key[16][8],tt[8];
    long tlen=0,i=0,ttlen=0;
    if((indata==NULL)||(outdata==NULL)||(key==NULL)
        ||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
        return 0;//传入参数错误
    p=indata;
    tlen=inlen/8;
    if(inlen%8!=0)
        tlen=tlen+1;
    tlen=tlen*8;//补整
    *outlen=tlen;//明文长度
    p1=outdata;tlen=inlen;
    for(i=0;i<8;i++)    tt[i]=0;
    genKey(key,s_key);//获取16个子密钥
    while(tlen>0)
    {//逐64位循环解密
        for(i=0;i<8;i++) tt[i]=0;
        ttlen=(tlen<8)?tlen:8;
        for(i=0;i<ttlen;i++)
            tt[i]=*(p+i);//取原数据
        comDES(tt,p1,s_key,g_DECRYPT_FLAG);
        p=p+8;p1=p1+8;tlen=tlen-8;
    }
    return 1;
}
//================随机生成密钥========================
void randKey(unsigned char key[8])
{//根随机生成一个64位(8字节)的密钥
    unsigned char i;
    for(i=0;i<8;i++)
        key[i]=rand() % 0x0100;
};
//==============DES-CBC============================================================
unsigned char enDES_cbc(unsigned char* indata,unsigned long inlen,unsigned char* key,\
        unsigned char* outdata,unsigned long* outlen,unsigned char* iv)
{//DES-CBC加密
    unsigned char *p,*p1,s_key[16][8],tt[8],ivt[8];
    long tlen=0,i=0,ttlen=0;
    if((indata==NULL)||(outdata==NULL)||(key==NULL)||(iv==NULL)
        ||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
        return 0;//传入参数错误
    for(i=0;i<8;i++)
        ivt[i]=iv[i];
    p=indata;
    tlen=inlen/8;
    if(inlen%8!=0)
        tlen=tlen+1;
    tlen=tlen*8;//补整
    *outlen=tlen;//密文长度
    p1=outdata;tlen=inlen;
    for(i=0;i<8;i++)    tt[i]=0;
    genKey(key,s_key);//获取16个子密钥
    while(tlen>0)
    {//逐64位循环加密
        for(i=0;i<8;i++) tt[i]=0;
        ttlen=(tlen<8)?tlen:8;
        for(i=0;i<ttlen;i++)
            tt[i]=*(p+i);//取原数据
        for(i=0;i<8;i++)//与iv异或
            tt[i]=tt[i] ^ ivt[i];
        comDES(tt,p1,s_key,g_ENCRYPT_FLAG);
        for(i=0;i<8;i++)
            ivt[i]=p1[i];
        p=p+8;p1=p1+8;tlen=tlen-8;
    }
    return 1;
}
unsigned char unDES_cbc(unsigned char* indata,unsigned long inlen,unsigned char* key,\
        unsigned char* outdata,unsigned long* outlen,unsigned char* iv)
{//DES-CBC解密
    unsigned char *p,*p1,s_key[16][8],tt[8],ivt[8];
    long tlen=0,i=0,ttlen=0;
    if((indata==NULL)||(outdata==NULL)||(key==NULL)||(iv==NULL)
        ||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
        return 0;//传入参数错误
    for(i=0;i<8;i++)
        ivt[i]=iv[i];
    p=indata;
    tlen=inlen/8;
    if(inlen%8!=0)
        tlen=tlen+1;
    tlen=tlen*8;//补整
    *outlen=tlen;//明文长度
    p1=outdata;tlen=inlen;
    for(i=0;i<8;i++)    tt[i]=0;
    genKey(key,s_key);//获取16个子密钥
    while(tlen>0)
    {//逐64位循环解密
        for(i=0;i<8;i++) tt[i]=0;
        ttlen=(tlen<8)?tlen:8;
        for(i=0;i<ttlen;i++)
            tt[i]=*(p+i);//取原数据
        comDES(tt,p1,s_key,g_DECRYPT_FLAG);
        for(i=0;i<8;i++)//与iv异或
            p1[i]=p1[i] ^ ivt[i];
        for(i=0;i<8;i++)
            ivt[i]=tt[i];
        p=p+8;p1=p1+8;tlen=tlen-8;
    }
    return 1;
}
unsigned char enY5DES(gYan5DES* y5)
{//研五DES加密
    int flag=0;
    if((y5==NULL)||(y5->len>g_YAN5_DES_LEN)
        ||(y5->len==0)||(y5->len%8!=0))
        return 0;//传入参数错误
    flag=enDES(y5->indata,y5->len,y5->key,y5->outdata,&(y5->len));
    if(flag) return 1;
    else return 0;
}
unsigned char unY5DES(gYan5DES* y5)
{//研五DES解密
    int flag=0;
    if((y5==NULL)||(y5->len>g_YAN5_DES_LEN)
        ||(y5->len==0)||(y5->len%8!=0))
        return 0;//传入参数错误
    flag=unDES(y5->indata,y5->len,y5->key,y5->outdata,&(y5->len));
    if(flag) return 1;
    else return 0;
}
void clrYan5DES(gYan5DES* y5)
{//清空结构
    int i=0;
    if(y5==NULL) return;
    for(i=0;i<g_YAN5_DES_LEN;i++)
    {
        y5->indata[i]=0;
        y5->outdata[i]=0;
    }
    for(i=0;i<8;i++)
        y5->key[i]=0;
    y5->len=0;
}


