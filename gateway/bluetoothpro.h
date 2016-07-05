#ifndef BLUETOOTHPRO_H_INCLUDED
#define BLUETOOTHPRO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "sys/select.h"


void bluetooth_init();
void bluet_connect();
void bluetooth_socket(bdaddr_t *ba);
void bluet_recv(void *arg);

#endif // BLUETOOTHPRO_H_INCLUDED

