// gcc -Og bomba.c -o bomba_ARM_2020 -no-pie -fno-guess-branch-probability 

#include <stdio.h>	// para printf(), fgets(), scanf()
#include <stdlib.h>	// para exit()
#include <string.h>	// para strncmp()
#include <sys/time.h>	// para gettimeofday(), struct timeval

#define SIZE 100
#define TLIM 60

char nomemires[] = "movimientohelicoidal\n";
int passsword = 0;

void boom(void){
	printf(	"\n"
		"*****************\n"
		"*** KABOOM!!! ***\n"
		"*****************\n"
		"\n");
	exit(-1);
}

void defused(void){
	printf(	"\n"
		"***************************************************\n"
		"*** Esta vez no exploto, pero habra una proxima ***\n"
		"***************************************************\n"
		"\n");
	exit(0);
}

int main(){
	char pw[SIZE];
	int  pc, n;

	struct timeval tv1,tv2;	// gettimeofday() secs-usecs
	gettimeofday(&tv1,NULL);

	do	printf("\nIntroduce la contraseña: ");
	while (	fgets(pw, SIZE, stdin) == NULL );

	if ( !strncmp(pw,pw,sizeof(pw)))
		printf ("\n Contraseña correcta \n");

	gettimeofday(&tv2,NULL);
	if    ( tv2.tv_sec - tv1.tv_sec > TLIM )
	    boom();

	passsword = (strlen(nomemires)-1)*50 + 1;

	do  {	printf("\nIntroduce el pin: ");
	 if ((n=scanf("%i",&pc))==0)
		scanf("%*s")    ==1;         }
	while (	n!=1 );

	if (pc != passsword || strncmp(pw,nomemires,sizeof(nomemires)) )
	    boom();

	gettimeofday(&tv1,NULL);
	if    ( tv1.tv_sec - tv2.tv_sec > TLIM )
	    boom();

	defused();
}