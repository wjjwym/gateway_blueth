#ifndef __RESOLVEJSON_H
#define __RESOLVEJSON_H

/*****************************************************************************
Copyright:     2015-2016, FireWork.
File name:     socket.c
Description:   ���ļ�������JSON���ݰ��Ľ����йصĺ��������ڹ��ߺ����ļ�
Author:        ShaoYang Yu
Version:       v3.0
Date:          2016-05-15
History:       2016-04-26 v1.0
               2016-05-8  v2.0
               2016-05-11 v3.0
*****************************************************************************/

#include"wgetDownload.h"
#include "cJSON.h"
#include"wgetDownload.h"
#include <stdio.h>
#include<stdlib.h>
#include<string.h>

/*json�����ݽ��մ洢�ṹ��*/
struct recv{
    //code:2112 2100
    char *filePath;
    char *fileType;
    //code:2100
    char *newVersion;
    char *hash ;
    //code:2113
    char *type;
    char *code;
};

int resolve_json(char* json);
char *get_json_stringValue(char *string_from_server, char *keyA, char *keyB);
int bytes_to_int(unsigned char* bytes);
char* unpack_json_packet(unsigned char* item);
void download_file(void* arg);
void clear_struct(struct recv re);
#endif
