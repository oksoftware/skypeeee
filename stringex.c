#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "stringex.h"

int SearchString(const char *haystack, size_t haystackLength, const char *needle){
	unsigned int i;
	size_t needleLength;
	
	needleLength = strlen(needle);
	
	if((haystackLength == 0) || (needleLength == 0) || (haystackLength < needleLength)){
		return -1;
	}
	
	for(i = 0; i < (haystackLength - needleLength + 1); i++){
		if(strncmp((haystack + i), needle, needleLength) == 0){
			return i;
		}
	}
	
	return -1;
}

char *AddString(char *dest, char *src){
	//FIXME!Implementation
	return dest;
}