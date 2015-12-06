#include<stdlib.h>
#include<stdio.h>

#define array_size 1024000 //for 4000KB
#define run 10

void create_delete(){
	int *int_p,i,cnt=1;
	while(cnt<=run){
	//1024*4B = 4KB
		int_p = malloc(array_size*sizeof(int)*cnt);
		sleep(5);
		for(i=cnt;i<array_size*cnt;i++)
		{
			int_p[i]=i;
		}
		//sleep(sleep_time);
		sleep(5);
		free(int_p);
		sleep(10);
		cnt++;
	}
}
int main(){
	sleep(15);
	printf("Started\n");
	create_delete();
	sleep(5);
	return 0;
}

