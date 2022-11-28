
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/unistd.h>             //提供POSIX操作系统API，如Unix的所有官方版本
#include <netinet/in.h>             //主要定义了一些类型
#include <arpa/inet.h>              //主要定义了格式转换函数
#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include "sdc_fd_tproxy.h"



#define SDC_CODE_200            200
#define SDC_CODE_400            400
#define SDC_CODE_401            401
#define SDC_CODE_403            403
#define SDC_CODE_500            500

#define SDC_URL_TPROXY_SERVER     	0x00
#define SDC_URL_TPROXY_CONNECTION 	0x01


// static int SDC_unix_server_create(int type, const char* path, int epoll_fd)
// {
//     struct epoll_event epoll_event;
//     struct event* event;
//     int fd;
//     struct sockaddr_un addr = {
//         .sun_family = AF_UNIX,
//     };
//     snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

//     (void)unlink(path);
//     fd = socket(AF_UNIX, type, 0);
//     if (fd == -1) {
//         printf("socket fail: %d, %m\n", errno);
//         return errno;
//     }

//     int nret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
//     if (nret == -1) {
//         printf("bind fail: %d, %m\n", errno);
//         close(fd);
//         return errno;
//     }

//     nret = type == SOCK_STREAM ? listen(fd, SOMAXCONN) : 0;
//     if (nret == -1) {
//         printf("listen fail:%d, %m\n", errno);
//         close(fd);
//         return errno;
//     }

//     event = malloc(sizeof(*event));
//     if (!event){
//         printf("event malloc fail\n");
//         close(fd);
//         return ENOMEM;
//     }

//     event->fd = fd;
//     event->destroy = event_destroy;
//     event->handle = type == SOCK_STREAM ? tcp_server_handle : connection_handle;

//     epoll_event.events = EPOLLIN;
//     epoll_event.data.ptr = event;
//     nret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
//     if (nret == -1) {
//         printf("epoll-ctl fail:%d, %m\n", errno);
//         close(fd);
//         event->destroy(event);
//         return errno;
//     }

//     return 0;
// }



int SDC_tproxy_server_register(int fd_tproxy, int domain, int type, const char* path, const char* filter)
{
    // struct sdc_tproxy_server info = { 
    //     .domain = domain,
    //     .type = type,
    // };

    struct sdc_tproxy_server info;
    memset(&info, 0, sizeof(info));
    info.domain = domain;
    info.type = type;




    // struct sdc_common_hdr = {
    //     .version = SDC_VERSION,
    //     .url = SDC_URL_TPROXY_SERVER,
    //     .method = SDC_METHOD_CREATE,
    //     .head_length = sizeof(hdr),
    //     .content_length = sizeof(info) + strlen(filter) +1,
    // };

    sdc_common_head_s hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.version = SDC_VERSION;
    hdr.url =SDC_URL_TPROXY_SERVER;
    hdr.method = SDC_METHOD_CREATE;
    hdr.head_length = sizeof(hdr);
    hdr.content_length = sizeof(info) + strlen(filter) +1;


    struct iovec iov[] = {
        {.iov_base = &hdr, .iov_len = sizeof(hdr) },
        {.iov_base = &info, .iov_len = sizeof(info) },
        {.iov_base = (char*)filter, .iov_len = strlen(filter) + 1 },
    };
    // int fd = open("/mnt/srvfs/tproxy.paas.sdc", O_RDWR);
    int nret;

    snprintf(info.addr, sizeof(info.addr), "%s", path);
    nret = writev(fd_tproxy, iov, sizeof(iov) / sizeof(iov[0]));
    if(nret == -1) {
        fprintf(stdout,"fd_tproxy writev  failed, path: %s, filter: %s", path, filter);
        return -1;
    }

    nret = read(fd_tproxy, &hdr, sizeof(hdr));
    if(nret == -1 || hdr.code != SDC_CODE_200)
    {
        fprintf(stdout,"fd_tproxy read failed, path: %s, code: %d", path, SDC_CODE_200);
        return -1;
    }
    return fd_tproxy;

}

int SocketConnect(int connfd,const std::string &ip, uint16_t port)
{

    if((connfd=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP))<0)
    {
        std::cerr<<"socket"<<std::endl;
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(connfd,(struct sockaddr*)&addr, sizeof(addr))<0)
    {
        std::cerr<<"connect"<<std::endl;
        return -1;
    }


        // /*---------------------------------------开始CONNECT连接----------------------------------*/

	uint8_t sendBuf[1024] = {0};
	uint8_t recvBuf[1024] = {0};
	int32_t msgLen = snprintf_s((char *)sendBuf, sizeof(sendBuf), sizeof(sendBuf) - 1, "CONNECT %s:%u \r\nHost:%s:%u\r\n\r\n",
		ip.c_str(), port, ip.c_str(), port);
	
    
	// 发送请求数据
	if(write(connfd, sendBuf, msgLen) < 0) {
		fprintf(stdout,"Send the connect message failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return -1;
	}

	// 接收返回数据
	if((msgLen=read(connfd, &recvBuf, sizeof(recvBuf))) < 0) {
		fprintf(stdout,"Receive the response failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return -1;
	}

	fprintf(stdout,"Receive the response from server successfully, recvmesg: %s",recvBuf);
    

    return 0;
}



int SDC_SocketConnect(int fd_tproxy,const std::string &ip, uint16_t port)
{
    struct sdc_tproxy_connection tproxy_addr = {
        .domain = AF_INET,
        .type = SOCK_STREAM,
    };

    sdc_common_head_s head;
	memset(&head, 0, sizeof(head));
	head.version = SDC_VERSION;
    head.url =1;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(tproxy_addr);

    struct iovec iov[] = {
        {.iov_base = &head, .iov_len = sizeof(head) },
        {.iov_base = &tproxy_addr, .iov_len = sizeof(tproxy_addr) },
    };

    printf("here3\n");
    int nret;
    (void)snprintf(tproxy_addr.addr, sizeof(tproxy_addr.addr), "%s:%u", ip.c_str(),port);
    fprintf(stdout,tproxy_addr.addr);
    nret = writev(fd_tproxy, iov, sizeof(iov) / sizeof(iov[0]));
    nret = read(fd_tproxy, &head, sizeof(head));
    printf("here2\n");
    if(nret == -1 || head.code != SDC_CODE_200 ||fd_tproxy==-1)
    {
        if(fd_tproxy > -1) close(fd_tproxy);
        fprintf(stdout,"Connect the  device failed, ip: %s, port: %u", ip.c_str(), port);
        return -1;
    }

    printf("here1\n");
    // /*---------------------------------------开始CONNECT连接----------------------------------*/

	uint8_t sendBuf[1024] = {0};
	uint8_t recvBuf[1024] = {0};
	int32_t msgLen = snprintf_s((char *)sendBuf, sizeof(sendBuf), sizeof(sendBuf) - 1, "CONNECT %s:%u \r\nHost:%s:%u\r\n\r\n",
		ip.c_str(), port, ip.c_str(), port);
	
    
	// 发送请求数据
	// if(write(fd_tproxy, sendBuf, msgLen) < 0) {
	// 	fprintf(stdout,"Send the connect message failed, errno: %d, errmsg: %s", errno, strerror(errno));
	// 	return -1;
	// }

	// 接收返回数据
	if((msgLen=read(fd_tproxy, &recvBuf, sizeof(recvBuf))) < 0) {
		fprintf(stdout,"Receive the response failed, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	fprintf(stdout,"Receive the response from server successfully, recvmesg: %s\n",recvBuf);
    
    return 0;
}



int SDC_SocketSendRequest(int fd_tproxy,const uint8_t msg[], int32_t msglen)
{
	// 发送请求数据
	if(write(fd_tproxy, msg, msglen) < 0) {
		fprintf(stdout,"Send the connect message failed");
		return -1;
	}
    return 0;
}


int SDC_SocketRecvResponse(int fd_tproxy,const uint8_t msg[], int32_t msglen)
{
	// 接收返回数据
	if((msglen=read(fd_tproxy, &msg, sizeof(msglen))) < 0) {
		fprintf(stdout,"Receive the response failed");
		return -1;
	}
    return 0;
}


void SDC_CloseSocket(int fd_tproxy)
{
	if(fd_tproxy > -1) close(fd_tproxy);
}
