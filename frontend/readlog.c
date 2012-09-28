/*
readlog.c - Reads the emumiga log continously and prints it on stdout.
            Handles interrupt by Ctrl-C

Public Domain

*/
#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/emumiga.h>
#include <dos/dos.h>

#define BUFSIZE 1000

int main(int argc, char *argv[])
{
	int cnt;
	char buf[BUFSIZE+1];

	SetSignal(0,SIGBREAKF_CTRL_C);
	while(1) {
		cnt=EM_getlog(buf,BUFSIZE);
		if(cnt<0) {
			fprintf(stderr,"An error occured while reading logs\n");
			exit(10);
		}
		buf[cnt]='\0';
		printf("%s",buf);

		if(SetSignal(0,SIGBREAKF_CTRL_C)&SIGBREAKF_CTRL_C)
			break;
	}

	exit(0);
}
