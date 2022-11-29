#include "sdc_proxy.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include "sdc_def_ext.h"

namespace HWYolov3App
{
const int32_t INVALID_SOCKET = -1;

HttpProxy::HttpProxy(void) : m_sock(INVALID_SOCKET)
{
}

HttpProxy::~HttpProxy(void)
{
	CloseSocket();
}

int32_t HttpProxy::CreateSocket(void)
{
    if((m_sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		LOG_ERROR("Create the proxy socket failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return IPROXY_ERROR;
	}

    LOG_DEBUG("Create the proxy socket successfully, socket: %d", m_sock);
    return IPROXY_OK;
}

void HttpProxy::CloseSocket(void)
{
	if (m_sock != INVALID_SOCKET) {
		close(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

int32_t HttpProxy::Connect(const std::string &ip, uint16_t port)
{
    //配置server_address
    struct sockaddr_un address;
    memset_s(&address, sizeof(address), 0, sizeof(address));
	address.sun_family = AF_UNIX;
    // 将地址绑定到nginx监听的域套接字/tmp/http_proxy_connect.socket
	strncpy_s(address.sun_path, sizeof(address.sun_path), HttpProxy::HTTP_PROXY_SOCK, sizeof(address.sun_path) - 1);

	if(connect(m_sock, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) < 0) {
		LOG_ERROR("Connect the peer device failed, ip: %s, port: %us, errno: %d, errmsg: %s", ip.c_str(), port, errno, strerror(errno));
		return IPROXY_ERROR;
	}

	/*---------------------------------------开始CONNECT连接----------------------------------*/
	// 要发给域套接字的请求信息
	// 消息内容如："CONNECT 172.31.0.3:8080 HTTP/1.1\r\nHost:172.31.0.3:8080\r\n\r\n"
	uint8_t sendBuf[1024] = {0};
	uint8_t recvBuf[1024] = {0};
	int32_t msgLen = snprintf_s((char *)sendBuf, sizeof(sendBuf), sizeof(sendBuf) - 1, "CONNECT %s:%u HTTP/1.1\r\nHost:%s:%u\r\n\r\n",
		ip.c_str(), port, ip.c_str(), port);
	
	// 发送请求数据
	if(write(m_sock, sendBuf, msgLen) < 0) {
		LOG_ERROR("Send the connect message failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return IPROXY_ERROR;
	}

	// 接收返回数据
	if((msgLen=read(m_sock, &recvBuf, sizeof(recvBuf))) < 0) {
		LOG_ERROR("Receive the response failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return IPROXY_ERROR;
	}

	LOG_DEBUG("Receive the response from server successfully, recvmesg: %s",recvBuf);
	return IPROXY_OK;
	/*---------------------------------------CONNECT 连接成功----------------------------------*/
}

int32_t HttpProxy::SendRequest(const uint8_t msg[], int32_t msglen)
{
	if(write(m_sock, msg, msglen) < 0) {
		LOG_ERROR("Send the request to the peer failed, errno: %d, msg: %s", errno, strerror(errno));
		return IPROXY_ERROR;
	}
	return IPROXY_OK;
}

int32_t HttpProxy::RecvResponse(uint8_t msg[], int32_t size, int32_t &msglen)
{
	if((msglen=read(m_sock, msg, size)) < 0) {
		LOG_ERROR("Receive the response from the peer failed, errno: %d, msg: %s", errno, strerror(errno));
		return IPROXY_ERROR;
	}
	return IPROXY_OK;
}
}