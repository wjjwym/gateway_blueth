#ifndef KEYINPUT_H_INCLUDED
#define KEYINPUT_H_INCLUDED

#include <stdint.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int listen_key (char* dev_event);

#endif // KEYINPUT_H_INCLUDED
