#include <string.h>

#include "stringex.h"

int searchString(size_t haystackOffset, const char *haystack, size_t haystackLength, const char *needle){
	unsigned int i;
	size_t needleLength;
	
	needleLength = strlen(needle);
	
	if((haystackLength == 0) || (needleLength == 0) || (haystackLength < needleLength)){
		return -1;
	}
	
	for(i = 0; i < (haystackLength - haystackOffset - needleLength + 1); i++){
		if(strncmp((haystack + haystackOffset + i), needle, needleLength) == 0){
			return i;
		}
	}
	
	return -1;
}
