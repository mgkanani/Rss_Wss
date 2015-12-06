#include<stdlib.h>
#include<stdio.h>

#define array_size 1024000 //for 4000KB
#define run 10

void noaccess(){
	int *int_p,i,cnt=1;
	while(cnt<=run){
		int_p = malloc(array_size*sizeof(int)*cnt);
		sleep(4);
		cnt++;
		free(int_p);
		sleep(8);
	}
}

int main(){
	sleep(15);
	printf("Started\n");
	noaccess();
	sleep(5);
	return 0;
}
