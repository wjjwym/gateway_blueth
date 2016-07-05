#include <stdio.h>
#include <stdlib.h>
#include "encryption.h"
#include "msgprocess.h"
#include "pthread.h"
#include "watchdog_pro.h"


/*************************************************
Function:       int main()
Description:    程序入口函数
Calls:          msgprocess.c(
                      void msgprocess()
                      )
                watchdog_pro.c(
                      void watchdog_pro()
                      )
Called By:      NONE
Input:          NONE
Output:         NONE
Return:         NONE
Others:         NONE
*************************************************/
int main()
{
    pthread_t msgprocess_thread;  /*创建线程描述符*/
    pthread_t watchdog_thread;  /*创建线程描述符*/
    if((pthread_create(&msgprocess_thread, NULL, msgprocess,NULL)) != 0)
    {
        perror("encryption thread create failed.");
        return 1;
    }

    if((pthread_create(&watchdog_thread, NULL, watchdog_pro,NULL)) != 0)
    {
        perror("process_monitor thread create failed.");
        return 1;
    }

    pthread_join(msgprocess_thread, NULL);
    pthread_join(watchdog_thread, NULL);

    return 0;
}
