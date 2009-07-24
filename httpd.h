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
	unsigned char *body;
} HTTPREQUEST;

int StartHTTPServerDaemon();
void FreeHTTPRequest(HTTPREQUEST *pHTTPRequest);
char *SearchHTTPRequestHeader(HTTPREQUEST *pHTTPRequest,char *needle);
