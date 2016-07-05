#include "watchdog_pro.h"


void watchdog_pro()
{
    int overcount = 0;
    while(1)
    {
        sleep(8);
        if(OVERTIMES<=overcount)
        {
            system("killall -9 gateway");
            system("/home/pi/project/gateway/bin/Debug/gateway");
            overcount=0;
        }

        if( (1==heartflg) && (overcount<OVERTIMES) )
        {
            overcount = 0;
            heartflg = 0;
        }
        else
        {
            overcount ++;
            printf("over times : %d \n", overcount);
        }

    }
}


void recvheart_process(char *msgdata, int msglen)
{
    if( (NULL!= msgdata) && (0!=msglen) )
    {
        heartflg=1;
    }
}
