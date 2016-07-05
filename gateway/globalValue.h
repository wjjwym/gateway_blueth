#ifndef __GLOBALVALUE_H
#define __GLOBALVALUE_H

#include <pthread.h>



#define ONE_CONFIG_MAX 128
#define CONF_FILENAME_ENCRP "/home/pi/project/gateway/gateway_config.bin"
#define CONF_FILENAME "/home/pi/project/gateway/gateway_config.txt"

enum MUTEX_NAME
{
    MUTEX_SOCKET,
    MUTEX_JSON,
    MUTEX_RECORD,
    MUTEX_MAX
};

pthread_mutex_t g_mutex[MUTEX_MAX];

enum CONFIG_NAME
{
    CONF_SERVERIP,
    CONF_SERVERPORT,
    CONF_INTERRUPTDELAY,
    CONF_DOWNLOADADDR,
    MAX_CONFIG
};


struct _CONFIGSTRING_
{
    char data[ONE_CONFIG_MAX];
    int len;
};

struct _CONFIGSTRING_ m_configlist[MAX_CONFIG];



#endif
