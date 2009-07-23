#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <process.h>
#include <malloc.h>

#include "stringex.h"
#include "httpd.h"

void __cdecl HTTPServerDaemonThread(void *p);
void __cdecl HTTPServerRequestThread(void *p);
HTTPREQUEST *HTTPRequestTokenizer(const char *pRequest, int requestLength);

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
	
	HTTPRequestTokenizer(recvBuf, recvSize);
	
	closesocket(clientSock);
	return;
}

//HTTP request tokenizer
HTTPREQUEST *HTTPRequestTokenizer(const char *pRequest, int requestLength){
	HTTPREQUEST *pHTTPRequest;
	HTTPREQUESTHEADER *pPreviousHTTPRequestHeader;
	int endOfLine, endOfRequestHeaders, current, nextLength;
	
	
	//Alloc HTTP request struct
	pHTTPRequest = (HTTPREQUEST *)malloc(sizeof(HTTPREQUEST));
	if(pHTTPRequest == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer.\n");
		return NULL;
	}
	
	pHTTPRequest->method = NULL;
	pHTTPRequest->path = NULL;
	pHTTPRequest->version = NULL;
	
	current = 0;
	
	//Start-line tokenize
	
	//Get the end of line
	endOfLine = searchString(0, pRequest, requestLength, "\r\n");
	if(endOfLine == -1){
		fprintf(stderr, "Can not tokenize HTTP request(end of first line).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	//Get request method
	nextLength = searchString(0, pRequest, endOfLine, " ");
	if(nextLength == -1){
		fprintf(stderr, "Can not tokenize HTTP request(method name).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	pHTTPRequest->method = (char *)malloc(nextLength + 1);
	if(pHTTPRequest->method == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer(method name).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	memset(pHTTPRequest->method, 0, (nextLength + 1));
	memmove(pHTTPRequest->method, pRequest, nextLength);
	
	current = nextLength + 1;
	
	//Get request path
	nextLength = searchString(current, pRequest, endOfLine, " ");
	if(nextLength == -1){
		fprintf(stderr, "Can not tokenize HTTP request(path).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	pHTTPRequest->path = (char *)malloc(nextLength + 1);
	if(pHTTPRequest->path == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer(path).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	memset(pHTTPRequest->path, 0, (nextLength + 1));
	memmove(pHTTPRequest->path, (pRequest + current), nextLength);
	
	current += nextLength + 1;
	
	//Get HTTP version
	nextLength = endOfLine - current;
	pHTTPRequest->version = (char *)malloc(nextLength + 1);
	if(pHTTPRequest->version == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer(HTTP version).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	memset(pHTTPRequest->version, 0, (nextLength + 1));
	memmove(pHTTPRequest->version, (pRequest + current), nextLength);
	
	fprintf(stderr, "%s", pHTTPRequest->version);
	
	current += nextLength + 2;
	
	//Get HTTP request headers
	
	//Get end of request headers
	endOfRequestHeaders = searchString(current, pRequest, requestLength, "\r\n\r\n");
	if(endOfRequestHeaders == -1){
		fprintf(stderr, "Can not tokenize HTTP request(end of request header).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	endOfRequestHeaders -= 2;
	
	//Alloc HTTP request header struct
	pPreviousHTTPRequestHeader = (HTTPREQUESTHEADER *)malloc(sizeof(HTTPREQUESTHEADER));
	if(pHTTPRequest == NULL){
		fprintf(stderr, "Can not alloc HTTP request header tokenize buffer.\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	//Get end of line
	endOfLine = searchString(current, pRequest, endOfRequestHeaders, "\r\n");
	if(endOfLine == -1){
		fprintf(stderr, "Can not tokenize HTTP request(end of request header line).\n");
		
		freeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	return pHTTPRequest;
}

void freeHTTPRequest(HTTPREQUEST *pHTTPRequest){
	free(pHTTPRequest->method);
	free(pHTTPRequest->path);
	free(pHTTPRequest->version);
	
	free(pHTTPRequest);
	return;
}