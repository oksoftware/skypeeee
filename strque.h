//Struct of string que
typedef struct tagSTRQUE{
	struct tagSTRQUE *next;
	char *str;
} STRQUE;

STRQUE *newStrQue();
STRQUE *popStrQue(STRQUE *pStrQue,char **str);
STRQUE *pushStrQue(STRQUE *pStrQue, char *str);
