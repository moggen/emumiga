/*
emumigastop.c - Shuts down emulation

Public Domain

*/
#include <stdlib.h>
#include <proto/emumiga.h>

int main(int argc, char *argv[])
{
	EM_shutdown();
	exit(0);
}
