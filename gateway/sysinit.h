#ifndef __SYSINIT_H
#define __SYSINIT_H

/*****************************************************************************
Copyright:     2016-2017, Firework.
File name:     sysinit.h
Description:   此文件主要完成家庭网关系统按键初始化的相关操作，使用了树莓派中
               的wiringPi库作为GPIO控制库
Author:        YU SHAO YANG
Version:       1.0
Date:          2016.04.02
History:       NONE
*****************************************************************************/
#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include "interrupt.h"
#include "globalValue.h"
#include "desEncryption.h"
#include "msgpro.h"
//#include "bluetoothpro.h"
#include "keyinput.h"

/*四个按键端口号为0，1，2，3（wPi编号下）*/
/*分别对D:\Technology Innovation\enterpreneur project\Intelligent network pro\gateway\sysinit.h应GPIO17，18，27，22（BCM编号下）*/
#define Key1 0
#define Key2 1
#define Key3 2
#define Key4 3

#define ENCRYPTION 0//1:encryption 0:not to do

extern void key_interrupt_1(void);
extern void key_interrupt_2(void);
extern void key_interrupt_3(void);
extern void key_interrupt_4(void);

void gateway_init(void);
void delay_us(unsigned int useconds);
void delay_ms(unsigned int mseconds);
void delay_s(unsigned int seconds);

void init_configfile();
char* get_file(const char *filename, int filelen);
unsigned int get_length(const char *filename);
void parse_config(char *p_conf_file);
struct _CONFIGSTRING_ find_string(const char *srcstr, const char *deststr);
char*  decryption_data(const char *p_conf_file, int filelen, int *convertlen);
void init_msgprocess();
void init_listen_key(char *dev_event);
#endif
