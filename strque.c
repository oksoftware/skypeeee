#include <stdio.h>
#include <malloc.h>

#include "strque.h"

//Create new string que
STRQUE *newStrQue(){
	STRQUE *pStrQue;
	
	pStrQue = (STRQUE *)malloc(sizeof(STRQUE));
	pStrQue->str = NULL;
	pStrQue->next = NULL;
	
	return pStrQue;
}

//Pop string que
STRQUE *popStrQue(STRQUE *pStrQue,char **str){
	STRQUE *pNextStrQue;
	
	pNextStrQue = pStrQue->next;
	if(pNextStrQue == NULL){
		return pStrQue;
	}
	
	*str = pNextStrQue->str;
	
	free(pStrQue);
	
	return pNextStrQue;
}

//Push string que
STRQUE *pushStrQue(STRQUE *pStrQue, char *str){
	STRQUE *pNextStrQue = NULL;
	
	pNextStrQue = (STRQUE *)malloc(sizeof(STRQUE));
	pNextStrQue->next = NULL;
	pNextStrQue->str = str;
	
	pStrQue->next = pNextStrQue;
	return pNextStrQue;
}
