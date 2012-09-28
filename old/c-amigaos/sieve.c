#include <stdio.h>
#include <stdlib.h>

#define NPRIMES  1000
#define FALSE 0
#define TRUE  1

int main(int argc, char *argv[]) {
  int n;
  int i,j;
  int flag;
  int primes[NPRIMES]; /*It will contain the primes smaller than n
                        *that we have already encountered*/
  int level;           /*1+Number of primes currently in PRIMES*/

  /*Introduction*/
  if(argc != 2) {
    printf("Usage: %s <number>\n",argv[0]);
    exit(10);
  }
  n = atoi(argv[1]);

  level = 0;

  /*Main body*/
  for(i=2;i<=n;i++) {
    for(j = 0, flag = TRUE; j<level && flag; j++)
      flag = (i%primes[j]);
    if (flag) { /*I is a prime */
      printf("%12d\n", i);
      if (level < NPRIMES)
	primes[level++] = i;
    }
  }
  exit(0);
}
