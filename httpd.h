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
} HTTPREQUEST;

int StartHTTPServerDaemon();
void freeHTTPRequest(HTTPREQUEST *pHTTPRequest);
