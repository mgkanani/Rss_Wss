#include<stdlib.h>
#include<stdio.h>

#define array_size 1024000 //for 4000KB
#define run 4

void access(){
	int *int_p,i,cnt=0,size=array_size*sizeof(int)*(run);
	int_p = malloc(size);
		sleep(5);
	printf("size=%d\n",size);
	while(cnt<run){
		for(i=cnt;i<array_size*run;i++)
		{
			int_p[i]=i;
		}
		sleep(5);
		cnt++;
	}
		sleep(5);
	free(int_p);
}

int main(){
	sleep(15);
	printf("Started\n");
	access();
	sleep(4);
	return 0;
}

