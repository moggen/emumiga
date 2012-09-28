/*
dump_mmap.c - Makes emumiga dump the memory map into the log

Public Domain

*/
#include <stdio.h>
#include <stdlib.h>
#include <proto/emumiga.h>

int main(int argc, char *argv[])
{
	EM_dump_mmap();

	exit(0);
}
