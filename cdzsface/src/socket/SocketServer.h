#ifndef __Data_H__
#define __Data_H__

#include <string>
#include "Base64.h"
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include "sdc.h"


class SocketServer
{
    public:
    SocketServer();
    virtual ~SocketServer();

};


#endif /* __Data_H__ */