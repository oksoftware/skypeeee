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
	HTTPREQUEST *pHTTPRequest = NULL;
	
	clientSock = (SOCKET)p;
	
	memset(recvBuf, 0,  sizeof(recvBuf));
	
	recvSize = recv(clientSock, recvBuf, sizeof(recvBuf), 0);
	if(recvSize >= sizeof(recvBuf)){
		fprintf(stderr, "HTTP request too long.\n");
		return;
	}
	
	pHTTPRequest = HTTPRequestTokenizer(recvBuf, recvSize);
	
	closesocket(clientSock);
	return;
}

//HTTP request tokenizer
HTTPREQUEST *HTTPRequestTokenizer(const char *pRequest, int requestLength){
	HTTPREQUEST *pHTTPRequest;
	HTTPREQUESTHEADER *pHTTPRequestHeader, **pPreviousHTTPRequestHeader;
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
	pHTTPRequest->header = NULL;
	pHTTPRequest->body = NULL;
	
	current = 0;
	
	//Start-line tokenize
	
	//Get the end of line
	endOfLine = SearchString(pRequest, requestLength, "\r\n");
	if(endOfLine == -1){
		fprintf(stderr, "Can not tokenize HTTP request(end of first line).\n");
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	//Get request method
	nextLength = SearchString(pRequest, endOfLine, " ");
	if(nextLength == -1){
		fprintf(stderr, "Can not tokenize HTTP request(method name).\n");
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	pHTTPRequest->method = (char *)malloc(nextLength + 1);
	if(pHTTPRequest->method == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer(method name).\n");
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	memset(pHTTPRequest->method, 0, (nextLength + 1));
	memmove(pHTTPRequest->method, pRequest, nextLength);
	
	current = nextLength + 1;
	
	//Get request path
	nextLength = SearchString((pRequest + current), (endOfLine - current), " ");
	if(nextLength == -1){
		fprintf(stderr, "Can not tokenize HTTP request(path).\n");
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	pHTTPRequest->path = (char *)malloc(nextLength + 1);
	if(pHTTPRequest->path == NULL){
		fprintf(stderr, "Can not alloc HTTP request tokenize buffer(path).\n");
		
		FreeHTTPRequest(pHTTPRequest);
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
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	memset(pHTTPRequest->version, 0, (nextLength + 1));
	memmove(pHTTPRequest->version, (pRequest + current), nextLength);
	
	current += nextLength + 2;
	
	//Get HTTP request headers
	
	//Get end of request headers
	endOfRequestHeaders = SearchString((pRequest + current), (requestLength - current), "\r\n\r\n");
	if(endOfRequestHeaders == -1){
		fprintf(stderr, "Can not tokenize HTTP request(end of request header).\n");
		
		FreeHTTPRequest(pHTTPRequest);
		return NULL;
	}
	
	endOfRequestHeaders += current + 2;
	
	pPreviousHTTPRequestHeader = &pHTTPRequest->header;
	
	while(1){
		if(current >= endOfRequestHeaders){
			*pPreviousHTTPRequestHeader = NULL;
			break;
		}
		
		//Alloc HTTP request header struct
		pHTTPRequestHeader = (HTTPREQUESTHEADER *)malloc(sizeof(HTTPREQUESTHEADER));
		if(pHTTPRequest == NULL){
			fprintf(stderr, "Can not alloc HTTP request header tokenize buffer.\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		*pPreviousHTTPRequestHeader = pHTTPRequestHeader;
		
		pHTTPRequestHeader->name = NULL;
		pHTTPRequestHeader->value = NULL;
		pHTTPRequestHeader->next = NULL;
		
		pPreviousHTTPRequestHeader = &pHTTPRequestHeader->next;
		
		//Get end of line
		endOfLine = SearchString((pRequest + current), (endOfRequestHeaders - current), "\r\n");
		if(endOfLine == -1){
			fprintf(stderr, "Can not tokenize HTTP request(end of request header line).\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		//Get HTTP request header name
		nextLength = SearchString((pRequest + current), endOfLine, ": ");
		if(nextLength == -1){
			fprintf(stderr, "Can not tokenize HTTP request(header name).\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		pHTTPRequestHeader->name = (char *)malloc(nextLength + 1);
		if(pHTTPRequestHeader->name == NULL){
			fprintf(stderr, "Can not alloc HTTP request tokenize buffer(header name).\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		memset(pHTTPRequestHeader->name, 0, (nextLength + 1));
		memmove(pHTTPRequestHeader->name, (pRequest + current), nextLength);
		
		current += nextLength + 2;
		
		//Get HTTP request header value
		nextLength = endOfLine - (nextLength + 2);
		
		pHTTPRequestHeader->value = (char *)malloc(nextLength + 1);
		if(pHTTPRequestHeader->value == NULL){
			fprintf(stderr, "Can not alloc HTTP request tokenize buffer(header value).\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		memset(pHTTPRequestHeader->value, 0, (nextLength + 1));
		memmove(pHTTPRequestHeader->value, (pRequest + current), nextLength);
		
		current += nextLength + 2;
	}
	
	current += 2;
	
	if((requestLength - current) != 0){
		pHTTPRequest->body = (char *)malloc(requestLength - current + 1);
		if(pHTTPRequest->body == NULL){
			fprintf(stderr, "Can not alloc HTTP request tokenize buffer(body).\n");
			
			FreeHTTPRequest(pHTTPRequest);
			return NULL;
		}
		
		memset(pHTTPRequestHeader->value, 0, (requestLength - current + 1));
		memmove(pHTTPRequestHeader->value, (pRequest + current), (requestLength - current));
	}
	
	return pHTTPRequest;
}

void FreeHTTPRequest(HTTPREQUEST *pHTTPRequest){
	HTTPREQUESTHEADER *pNextHTTPRequestHeader, *pHTTPRequestHeader;
	free(pHTTPRequest->method);
	free(pHTTPRequest->path);
	free(pHTTPRequest->version);
	
	//Free HTTPREQUESTHEADERs
	pHTTPRequestHeader = pHTTPRequest->header;
	
	while(1){
		if(pHTTPRequestHeader != NULL){
			free(pHTTPRequestHeader->name);
			free(pHTTPRequestHeader->value);
			
			pNextHTTPRequestHeader = pHTTPRequestHeader->next;
			
			free(pHTTPRequestHeader);
		}else{
			break;
		}
		pHTTPRequestHeader = pNextHTTPRequestHeader;
	}
	
	free(pHTTPRequest->header);
	free(pHTTPRequest->body);
	free(pHTTPRequest);
	
	return;
}

char *SearchHTTPRequestHeader(HTTPREQUEST *pHTTPRequest,char *needle){
	HTTPREQUESTHEADER *pHTTPRequestHeader = NULL;
	
	pHTTPRequestHeader = pHTTPRequest->header;
	
	while(1){
		if(pHTTPRequestHeader == NULL){
			return NULL;
		}
		
		if(strcmp(pHTTPRequestHeader->name, needle) == 0){
			return pHTTPRequestHeader->value;
		}
		
		pHTTPRequestHeader = pHTTPRequestHeader->next;
	}
}
