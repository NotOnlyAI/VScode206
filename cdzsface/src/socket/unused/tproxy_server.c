#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


struct sdc_tproxy_server
{
	int32_t domain;
	int32_t type;
	char addr[128];
	char filter[0];
};

struct sdc_tproxy_connection
{
	int domain;
	int type;
	char addr[128];
};

struct sdc_common_head
{
        uint16_t        version;
        uint8_t         url_ver;
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint8_t         method: 7;
        uint8_t         response: 1;
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        uint8_t         response: 1;
        uint8_t         method: 7;
#else
#error "unknown __BYTE_ORDER__"
#endif
#else
#error "don't define __BYTE_ORDER__ or __ORDER_LITTLE_ENDIAN__ or __ORDER_BIG_ENDIAN__"
#endif
        uint16_t        url;
        uint16_t        code;
        uint16_t        head_length;
        uint16_t        trans_id;
        uint32_t        content_length;
};


#define SDC_VERSION            0x5331
#define SDC_URL_HARDWARE_ID    100

#define SDC_METHOD_CREATE       1
#define SDC_METHOD_GET          2
#define SDC_METHOD_UPDATE       3
#define SDC_METHOD_DELETE       4

#define SDC_CODE_200            200
#define SDC_CODE_400            400
#define SDC_CODE_401            401
#define SDC_CODE_403            403
#define SDC_CODE_500            500



#define SDC_URL_TPROXY_SERVER     	0x00
#define SDC_URL_TPROXY_CONNECTION 	0x01

/**
* type: SOCK_STREAM | SOCK_DGRAM
* RETURN: 返回到proxy服务的通信句柄，不要关闭，一直保留，进程退出会自动回收
* 返回之后有客户端连接到tproxy端口，根据首包匹配filter成功后会将消息转发给path对应的socket
* 用户应该先调用unix_server,成功后再调用tproxy_server_register
*/
static int tproxy_server_register(int domain, int type, const char* path, const char* filter)
{
    struct sdc_tproxy_server info = { 
        .domain = domain,
        .type = type,
    };
    struct sdc_common_head hdr = {
        .version = SDC_VERSION,
        .url = SDC_URL_TPROXY_SERVER,
        .method = SDC_METHOD_CREATE,
        .head_length = sizeof(hdr),
        .content_length = sizeof(info) + strlen(filter) +1,
    };
    struct iovec iov[] = {
        {.iov_base = &hdr, .iov_len = sizeof(hdr) },
        {.iov_base = &info, .iov_len = sizeof(info) },
        {.iov_base = (char*)filter, .iov_len = strlen(filter) + 1 },
    };
    int fd = open("/mnt/srvfs/tproxy.paas.sdc", O_RDWR);
    int nret;

    snprintf(info.addr, sizeof(info.addr), "%s", path);
    if( fd == -1) goto fail;

    nret = writev(fd, iov, sizeof(iov) / sizeof(iov[0]));
    if(nret == -1) goto fail;

    nret = read(fd, &hdr, sizeof(hdr));
    if(nret == -1 || hdr.code != SDC_CODE_200) goto fail;

    return fd;
fail:
    if(fd > -1) close(fd);
    return -1;
}

struct event {
    void (*handle)(struct event* event, unsigned int events, int epoll_fd);
    void (*destroy)(struct event* event);
    int fd;
};

static int unix_server_create(int type , const char* addr, int epoll_fd);

static void help(int argc, char* argv[])
{
    printf("%s [options]:\n"
        "-a [unix domain socket path]\n"
        "-d [unix | inet], only support unix in container\n"
        "-t [udp | tcp]\n"
        "-f [data filter]\n"
        "-h help\n", argv[0]);
    exit(0);
}

int main(int argc, char* argv[])
{
    const char* addr = "";
    const char* filter = "";
    int domain = 0;
    int type = 0;
    int epoll_fd ;
    struct epoll_event epoll_event;

    int opt;
    while((opt = getopt(argc, argv, ":a:d:t:f:h")) != -1) 
    {
    switch(opt) {
    case 'a':
        addr = optarg;
        break;
    case 'd':
        if(!strcmp(optarg, "unix")) {
            domain = AF_UNIX;
        }else {
            domain = AF_INET;
        }
        break;
    case 't':
        if(!strcmp(optarg, "udp")) {
            type = SOCK_DGRAM;
        }else {
            type = SOCK_STREAM;
        }
        break;
    case 'f':
        filter = optarg;
        break;
    case 'h':
        help(argc,argv);
        break;
    }
    }

    int fd = tproxy_server_register(domain, type, addr, filter);
    if (fd == -1) {
        printf("tproxy_server_register fail: %m\n");
        return -1;
    }

    epoll_fd = epoll_create(1);
    if (epoll_fd == -1){
        printf("epoll_fd create fail: %m\n");
        return errno;
    }

    int nret = unix_server_create(type, addr, epoll_fd);
    if (nret) {
        printf("unix_server_create: %d, %m\n", nret);
        return nret;
    }

    for (;;) {
        nret = epoll_wait(epoll_fd, &epoll_event, 1, -1);
        if (nret > 0) {
            struct event* event = epoll_event.data.ptr;
            event->handle(event, epoll_event.events, epoll_fd);
        }
    }
    close(fd);
    close(epoll_fd);
}

static void event_destroy(struct event* event)
{
    free(event);
}

static void connection_handle(struct event* event, unsigned int events, int epoll_fd)
{
    struct epoll_event epoll_event;
    char buf[1024];
    int nret;

    nret = read(event->fd, buf, sizeof(buf));
    if (nret > 0) {
        printf("read %d bytes:%.*s\n", nret, nret, buf);
        write(event->fd, buf, nret);
    }else if (nret == 0) {
        printf("peer close conneciton\n");
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event->fd, &epoll_event);
        event->destroy(event);
    }else {
        printf("read fail:%d, %m\n", errno);
        switch(nret){
            case EAGAIN:
            case EINTR:
                break;
            default:
                printf("close conneciton\n");
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event->fd, &epoll_event);
                event->destroy(event);
                break;
        }
    }
}

static void tcp_server_handle(struct event* event, unsigned int events, int epoll_fd)
{
    int c ;
    struct epoll_event epoll_event;
    struct event* conn_event = 0;
    c = accept(event->fd, 0, 0);
    if (c == -1) {
        printf("accept fail: %d, %m\n", errno);
        return;
    }

    conn_event = malloc(sizeof(*conn_event));
    if (!conn_event) {
        printf("malloc event fail\n");
        close(c);
        return ;
    }

    conn_event->fd = c;
    conn_event->handle = connection_handle;
    conn_event->destroy = event_destroy;

    epoll_event.events = EPOLLIN;
    epoll_event.data.ptr = conn_event;
    int nret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c, &epoll_event);
    if (-1 == nret) {
        printf("epoll_ctl fail: %d, %m\n", errno);
        close(c);
        return;
    }
    printf("new connection\n");
}

static int unix_server_create(int type, const char* path, int epoll_fd)
{
    struct epoll_event epoll_event;
    struct event* event;
    int fd;
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

    (void)unlink(path);
    fd = socket(AF_UNIX, type, 0);
    if (fd == -1) {
        printf("socket fail: %d, %m\n", errno);
        return errno;
    }

    int nret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (nret == -1) {
        printf("bind fail: %d, %m\n", errno);
        close(fd);
        return errno;
    }

    nret = type == SOCK_STREAM ? listen(fd, SOMAXCONN) : 0;
    if (nret == -1) {
        printf("listen fail:%d, %m\n", errno);
        close(fd);
        return errno;
    }

    event = malloc(sizeof(*event));
    if (!event){
        printf("event malloc fail\n");
        close(fd);
        return ENOMEM;
    }

    event->fd = fd;
    event->destroy = event_destroy;
    event->handle = type == SOCK_STREAM ? tcp_server_handle : connection_handle;

    epoll_event.events = EPOLLIN;
    epoll_event.data.ptr = event;
    nret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
    if (nret == -1) {
        printf("epoll-ctl fail:%d, %m\n", errno);
        close(fd);
        event->destroy(event);
        return errno;
    }

    return 0;
}