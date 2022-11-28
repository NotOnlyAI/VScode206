#ifndef __SDC_PROXY_H__
#define __SDC_PROXY_H__

#include <string>


class HttpProxy
{
public:
    enum {
        IPROXY_ERROR    = -1,
        IPROXY_OK       = 0,
    };

    virtual ~HttpProxy(void);
    int32_t CreateSocket(void);
    void CloseSocket(void);
    int32_t Connect(const std::string &ip, uint16_t port);
    int32_t SendRequest(const uint8_t msg[], int32_t msglen);
    int32_t RecvResponse(uint8_t msg[], int32_t size, int32_t &msglen);

protected:
    HttpProxy(void);

private:
    HttpProxy(const HttpProxy&);
    HttpProxy& operator=(const HttpProxy&);
    HttpProxy(const HttpProxy&&);
    HttpProxy& operator=(const HttpProxy&&);

    int32_t m_sock;

    // nginx监听的域套接字/tmp/http_proxy_connect.socket
    static constexpr const char *HTTP_PROXY_SOCK = "/tmp/http_proxy_connect.socket";
};


#endif /* __SDC_PROXY_H__ */