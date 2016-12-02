#pragma once
#include "MythConfig.hh"
#ifdef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <Winsock2.h>
#pragma comment(lib,"ws2_32")
#else
#include <wchar.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif
#include <string.h>
#include <thread>
class MythSocket
{
public:
	int socket_SendStr(const char* str, int length = -2);
	int socket_ReceiveData(char* recvBuf, int recvLength, int timeout = 500);
	int socket_CloseSocket();
	int socket_ReceiveDataLn2(char* recvBuf, int recvLength, char* lnstr);
	static MythSocket* CreateNew(const char* ip, int port){
		return new MythSocket(ip, port);
	}
	static MythSocket* CreateNew(int sockfd){
		return new MythSocket(sockfd);
	}
	~MythSocket();
	int active;
	int isPush;
	void* addtionaldata;
	void* data;
private:
	struct sockaddr_in addrSrv;
	int _sockfd;
	bool isrunning;
	//char* downbuffer;
	int downlength;
	int maxlength;
protected:
	MythSocket(const char* ip, int port);
	MythSocket();
	MythSocket(int sockfd);
	int wait_on_socket(int sockfd, int for_recv, long timeout_ms);
};
