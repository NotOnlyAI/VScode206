
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "SocketData.h"

#include "sdc_fd_tproxy.h"
#include "common_defs.h"





SocketDataSender::SocketDataSender()
{
}

SocketDataSender::~SocketDataSender()
{
	SDC_CloseSocket(m_socket);
}


int32_t SocketDataSender::CloseSocket()
{
	SDC_CloseSocket(m_socket);
    return 0;
}

int32_t SocketDataSender::init(int proxy_type)
{
    m_ip.assign("192.168.15.154");
    m_port=1234;
    m_proxy_type=proxy_type;

    m_socket_state=ConnectRequest();
    return 0;
}


int32_t SocketDataSender::ConnectRequest()
{
    if (m_proxy_type==0) //sdc_tcp
    {   
        m_socket= open("/mnt/srvfs/tproxy.paas.sdc", O_RDWR);
        if((SDC_SocketConnect(m_socket, m_ip, m_port)) < 0) {
		    LOG_ERROR("Create the proxy socket failed, ip: %s, port: %u", m_ip.c_str(), m_port);
		    return -1;
	    }
        LOG_DEBUG("Create the proxy socket(TCP) successfully, ip: %s, port: %u", m_ip.c_str(), m_port);
        return 0;
    }

    if(m_proxy_type==1)
    {
        int len,result;
	    struct sockaddr_un address;
	    if((m_socket = socket(AF_UNIX, SOCK_STREAM, 0))==-1)
	    {
		    LOG_ERROR("Create the proxy socket(HTTP) failed");
		    return -1;
	    }
        address.sun_family = AF_UNIX;
	    strcpy(address.sun_path, "/tmp/http_proxy_connect.socket");
	    len = sizeof(address);
	    result = connect(m_socket, (struct sockaddr *)&address, len);
	    if(result == -1) 
	    {
		    LOG_ERROR("Connect the proxy socket(HTTP) failed");
		    return -1;
	    }

        /*---------------------------------------开始CONNECT连接----------------------------------*/
        int byte;
        static const char* const connect_format = "CONNECT %s:%d HTTP/1.1\r\nHost:%s:%d\r\n\r\n";
	    char send_buf_connect[512];
        int msgLen = snprintf(send_buf_connect, sizeof(send_buf_connect), connect_format, m_ip.c_str(), m_port, m_ip.c_str(), m_port);
        // if msgLen>512
        // int32_t msgLen = snprintf_s((char *)send_buf_connect, sizeof(send_buf_connect), sizeof(send_buf_connect) - 1, "CONNECT %s:%u \r\nHost:%s:%u\r\n\r\n",
		// ip.c_str(), port, ip.c_str(), port);
	    
        if((byte=write(m_socket, send_buf_connect, strlen(send_buf_connect)))==-1)
        {
            LOG_ERROR("Write the proxy socket(HTTP) failed");
            return -1;
        }
        LOG_DEBUG("Send connectUrl successfully, connectUrl:%s,lenth:%d",send_buf_connect,strlen(send_buf_connect));

        char ch_recv_connect[512];
        if((byte=read(m_socket,&ch_recv_connect,512))==-1||!strstr(ch_recv_connect, "200"))
        {
            LOG_ERROR("Read the proxy socket(HTTP) failed");
            return -1;
        }
        LOG_DEBUG("Receive from server data successfully, ch_recv_connect:%s",ch_recv_connect);
        
        return 0;
    }
    // if((SocketConnect(m_socket, ip, port)) < 0) {
	// 	LOG_ERROR("Create the proxy socket failed, ip: %s, port: %u", ip.c_str(), port);
	// 	return -1;
	// }
    
}

int32_t SocketDataSender::GetRequestTest()
{   
    if(m_proxy_type==1)
    {
        int len,result;
	    struct sockaddr_un address;
        // //创建socket，指定通信协议为AF_UNIX,数据方式SOCK_STREAM
        // if((m_socket = socket(AF_UNIX, SOCK_STREAM, 0))==-1)
        // {
        // 	LOG_ERROR("Create the proxy socket(HTTP) failed");
        // 	return -1;
        // }
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, "/tmp/http_proxy_connect.socket");
        len = sizeof(address);
        result = connect(m_socket, (struct sockaddr *)&address, len);
        if(result == -1) 
        {
            LOG_ERROR("Connect the proxy socket(HTTP) failed");
            return -1;
        }
    
         /*---------------------------------------开始Get连接----------------------------------*/
        int byte;
        static const char* const get_format = "GET /index HTTP/1.1\r\nProxy-Connection:keep-alive\r\nHost:%s:%d\r\n\r\n";
        char send_buf_connect[512];
        int msgLen = snprintf(send_buf_connect, sizeof(send_buf_connect), get_format, m_ip.c_str(),m_port); 
        if((byte=write(m_socket, send_buf_connect, strlen(send_buf_connect)))==-1)
        {
            LOG_ERROR("GetRequestTest failed");
            return -1;
        }
        LOG_DEBUG("GetRequestTest successfully, connectUrl:%s,lenth:%d",send_buf_connect,strlen(send_buf_connect));

        char ch_recv_connect[512];
        if((byte=read(m_socket,&ch_recv_connect,512))==-1||!strstr(ch_recv_connect, "200"))
        {
            LOG_ERROR("GetRequestTest failed");
            return -1;
        }
        LOG_DEBUG("Receive from server data successfully, ch_recv_connect:%s",ch_recv_connect); 
        return 0;

    }
    return 0;
    
}




int32_t SocketDataSender::send_test()
{

    std::string msg="testtesttesttestesteststsetststeestsetstsetstsetetetstetsetestetetsetsetsetsts";
    const char *cstr = msg.c_str();
    uint8_t sendBuf[1024] = {0};
    int32_t msgLen = snprintf_s((char *)sendBuf, sizeof(sendBuf), sizeof(sendBuf) - 1, cstr);
    if(SDC_SocketSendRequest(m_socket,sendBuf,msgLen)<0)
    {
        LOG_ERROR("Send image data failed, imagelenth:%u",msg.length());
		return -1;
    }
    LOG_DEBUG("Send image data successfully, imagelenth:%u",msg.length());
    return 0;
}


int32_t SocketDataSender::sendMsg(const uint8_t *sendBuf,int32_t msgLen)
{
    
    if(SDC_SocketSendRequest(m_socket,sendBuf,msgLen)<0)
    {
        LOG_ERROR("Send Msg failed, Msglenth:%u",msgLen);
		return -1;
    }
    // LOG_DEBUG("Send Msg successfully, Msglenth:%u",msgLen); 
    return 0;
}