/*
emumigarunwb.c - Bootstrap program for running the emulator from Wanderer

Public Domain

*/

#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/emumiga.h>
#include <workbench/startup.h>

int main(int argc, char *argv[])
{
	struct WBStartup *wbmsg;
	BPTR win;
	char buffer[2000];
	int retc, err;

	if(argc!=0) {
		fprintf(stderr,"%s may only be called from Wanderer\n",argv[0]);
		exit(100);
	}

	wbmsg = (struct WBStartup *)argv;

	if(wbmsg->sm_NumArgs != 2) {
		win = Open("con:0/0/640/400/emumigawb/CLOSE/WAIT",MODE_NEWFILE);
		if(win == NULL)
			exit(100);
		FPrintf (win,"Error: sm_NumArgs is not 2\n",buffer);
		Close (win);
		exit(10);
	}

	NameFromLock (wbmsg->sm_ArgList[1].wa_Lock,buffer,256);
	AddPart (buffer,wbmsg->sm_ArgList[1].wa_Name,256);

	retc = 0;

	err = EM_run(buffer,"",&retc,10*1024);

	if(err != 0) {
		win = Open("con:0/0/640/400/emumigawb/CLOSE/WAIT",MODE_NEWFILE);
		if(win == NULL)
			exit(100);
		FPrintf(win,"EM_run returned error code: %d\n",err);
		Close (win);
		exit(100);
	}

	exit(retc);
}
