#ifndef DAEMON_INIT_H
#define DAEMON_INIT_H

#include "avs.h"
#include <syslog.h>
#define MAXFD 64

extern int daemon_proc;

int daemon_init(const char * pname, int facility);
#endif // DAEMON_INIT_H
