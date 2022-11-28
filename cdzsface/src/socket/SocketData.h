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


class SocketDataSender
{
    public:
    SocketDataSender();
    virtual ~SocketDataSender();
    int32_t init(int proxy_type);
    int32_t GetRequestTest();
    int32_t ConnectRequest();
    int32_t send_test();
    int32_t sendMsg(const uint8_t *sendBuf,int32_t msgLen);
    int32_t CloseSocket();


    int m_proxy_type;
    int m_socket_state;
    std::string m_ip;
    uint16_t m_port;
private:
    int32_t m_socket;



    



//     // nginx监听的域套接字/tmp/http_proxy_connect.socket
//     static constexpr const char *HTTP_PROXY_SOCK = "/tmp/http_proxy_connect.socket";
};


#endif /* __Data_H__ */