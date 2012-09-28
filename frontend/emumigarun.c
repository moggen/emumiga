/*
emumigarun.c - Main bootstrap program for running the emulator

Public Domain

*/

#include <stdio.h>
#include <stdlib.h>
#include <proto/emumiga.h>

int main(int argc, char *argv[])
{
	int i, retc, err;
	char argbuf[2000],*buf,*cur;

	if(argc<2) {
		fprintf(stderr,"Usage: %s <amiga exe> [<arg> ...]\n",argv[0]);
		exit(100);
	}

	buf=argbuf;
	for(i=2;i<argc;i++) {
		if(i>2) *buf++ = ' ';
		cur = argv[i];
		while(*cur != '\0')
			*buf++ = *cur++;
	}
	*buf = '\0';

	retc = 0;

	err = EM_run(argv[1],argbuf,&retc,10*1024);

	if(err != 0) {
		fprintf(stderr,"EM_run returned error code: %d\n",err);
		exit(100);
	}

	exit(retc);
}
