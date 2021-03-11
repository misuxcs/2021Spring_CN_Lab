#include<stdio.h>

char checksum(char type, char identifier, char sequence){
	char sum1 = type + identifier + sequence;
	char higherbit = sum1 >> 4;
	char lowerbit = sum1 << 4;
	char sum2 = higherbit + lowerbit;
	char complement = ~sum2;
	return complement;
}
