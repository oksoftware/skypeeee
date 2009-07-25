#include <malloc.h>
#include "httpd.h"
#include "stringex.h"

char *RequestCallback(HTTPREQUEST *pHTTPRequest){
	char *pHTTPResponse = NULL;
	pHTTPResponse = AddString(pHTTPResponse,
		"HTTP/1.x 200 OK\r\n"
		"Server: Skypeeee/0.5\r\n"
		"Content-Length: 12\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plan\r\n"
		"\r\n"
		"HELLO WORLD.");
	
	
	return pHTTPResponse;
}
