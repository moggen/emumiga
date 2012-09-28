#include <stdio.h>
#include <stdlib.h>
#include <proto/emumiga.h>

int main(int argc, char *argv[])
{
	if(argc!=2){
		fprintf(stderr,"Usage: %s <string>\n",argv[0]);
		exit(10);
	}

	EM_putlog(argv[1]);
	EM_putlog("\n");

	exit(0);
}
