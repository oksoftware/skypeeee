#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <process.h>
#include <malloc.h>

#include "httpd.h"


typedef struct tagHTTPREQUEST{
	unsigned char *method;
	unsigned char *path;
	unsigned char *version;
} HTTPREQUEST;

void __cdecl HTTPServerDaemonThread(void *p);
void __cdecl HTTPServerRequestThread(void *p);
HTTPREQUEST *HTTPRequestTokenizer(unsigned char *request, unsigned int requestLength);

//Starting HTTP server daemon thread
int StartHTTPServerDaemon(){
	if(_beginthread(&HTTPServerDaemonThread, 0, NULL) == (unsigned)-1){
		fprintf(stderr, "HTTP server daemon thread starting error.\n");
		return -1;
	}
	
	
	return 0;
}

//HTTP server daemon thread
void __cdecl HTTPServerDaemonThread(void *p){
	WSADATA wsaData;
	SOCKET listenSock, clientSock;
	struct sockaddr_in addr, clientAddr;
	int length;
	BOOL YES = 1;
	//struct sockaddr_in 
	
	//Startup Winsock
	if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0){
		fprintf(stderr, "Winsock startup error.\n");
		return;
	}
	
	//Create socket
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if(listenSock == INVALID_SOCKET){
		fprintf(stderr, "Invalid socket.\n");
		return;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	
	//Set socket option
	setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&YES, sizeof(YES));
	
	if(bind(listenSock, (struct sockaddr *)&addr, sizeof(addr)) != 0){
		fprintf(stderr, "Winsock binding error.\n");
		return;
	}
	
	if(listen(listenSock, 5) != 0){
		fprintf(stderr, "Winsock listening error.\n");
		return;
	}
	
	length = sizeof(clientAddr);
	
	while(1){
		clientSock = accept(listenSock, (struct sockaddr *)&clientAddr, &length);
		if(clientSock == INVALID_SOCKET){
			fprintf(stderr, "Winsock accepting error.\n");
			break;
		}
		
		//Starting HTTP server request thread
		if(_beginthread(&HTTPServerRequestThread, 0, (void *)clientSock) == (unsigned)-1){
			fprintf(stderr, "HTTP server request thread starting error.\n");
			break;
		}
	}
	
	WSACleanup();
	
	return;
}


//HTTP server daemon request thread
void __cdecl HTTPServerRequestThread(void *p){
	char recvBuf[30*1024];
	int recvSize = 0;
	SOCKET clientSock;
	clientSock = (SOCKET)p;
	
	memset(recvBuf, 0,  sizeof(recvBuf));
	
	recvSize = recv(clientSock, recvBuf, sizeof(recvBuf), 0);
	if(recvSize >= sizeof(recvBuf)){
		fprintf(stderr, "HTTP request too long.\n");
		return;
	}
	
	fprintf(stderr, "%s", recvBuf);
	
	closesocket(clientSock);
	return;
}

//HTTP request tokenizer
HTTPREQUEST *HTTPRequestTokenizer(unsigned char *request, unsigned int requestLength){
	HTTPREQUEST *pHTTPRequest;
	
	pHTTPRequest = (HTTPREQUEST *)malloc(sizeof(HTTPREQUEST));
	if(pHTTPRequest == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer.\n");
		return NULL;
	}
	
	//!FIXME
	
	return pHTTPRequest;
}

