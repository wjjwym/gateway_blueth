#ifndef WATCHDOG_PRO_H_INCLUDED
#define WATCHDOG_PRO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/wait.h>

#define OVERTIMES 3
static int heartflg = 0;
void watchdog_pro();
void recvheart_process(char *msgdata, int msglen);

#endif // WATCHDOG_PRO_H_INCLUDED
