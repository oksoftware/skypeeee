typedef struct tagHTTPREQUESTHEADER{
	char *name;
	char *value;
	struct tagHTTPREQUESTHEADER *next;
}HTTPREQUESTHEADER;

typedef struct tagHTTPREQUEST{
	char *method;
	char *path;
	char *version;
	HTTPREQUESTHEADER *header;
	char *body;
} HTTPREQUEST;

int StartHTTPServerDaemon(unsigned short port, char *(*pRequestCallback)(HTTPREQUEST *pHTTPRequest));
void FreeHTTPRequest(HTTPREQUEST *pHTTPRequest);
char *SearchHTTPRequestHeader(HTTPREQUEST *pHTTPRequest,char *needle);
