#ifndef __SDC_fd_tPROXY_H__
#define __SDC_fd_tPROXY_H__


#include <string>
#include <iostream>

#ifdef __cplusplus
extern "C"{
#endif

#include "sdc.h"

struct sdc_tproxy_connection
{
    int domain;
    int type;
    char addr[128];
};

struct sdc_tproxy_server
{
	int32_t domain;
	int32_t type;
	char addr[128];
	char filter[0];
};

extern int SDC_SocketConnect(int fd_tproxy,const std::string &ip, uint16_t port);
extern int SDC_SocketSendRequest(int fd_tproxy,const uint8_t msg[], int32_t msglen);
extern int SDC_SocketRecvResponse(int fd_tproxy,const uint8_t msg[], int32_t msglen);
extern void SDC_CloseSocket(int fd_tproxy);
extern int SocketConnect(int &connfd,const std::string &ip, uint16_t port);

extern int SDC_tproxy_server_register(int fd_tproxy, int domain, int type, const char* path, const char* filter);

#ifdef __cplusplus
}
#endif



#endif /* __SDC_fd_tPROXY_H__ */