#ifndef ENCRYPTION_H_INCLUDED
#define ENCRYPTION_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "desEncryption.h"

#define PATH_CONFIGFILE "/home/pi/project/daemonpro/gateway_config.bin"

char* encryption_data(const char *p_conf_file, int filelen, int *convertlen);
void process_configfile(const char *p_conf_file, int filelen);
void save_configfile(char* convertbuf, int filelen);

#endif // ENCRYPTION_H_INCLUDED
