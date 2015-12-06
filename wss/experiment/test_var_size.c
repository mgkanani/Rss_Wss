#include<stdlib.h>
#include<stdio.h>

#define array_size 1024000 //for 4000KB
#define run 10
#define sleep_time 2

void create_delete(){
	int *int_p,i,cnt=1;
	while(cnt<=run){
	//1024*4B = 4KB
		//will cause memory allocation.
		int_p = malloc(array_size*sizeof(int)*cnt);
		sleep(5);
		for(i=0;i<array_size*cnt;i=i+1024)
		{
			int_p[i]=i;
			//printf("Check\n");
		}
		sleep(4);
		for(i=0;i<array_size*cnt;i=i+255)
		{
			int_p[i]=i;
			//printf("Check\n");
		}
		sleep(4);
		for(i=0;i<array_size*cnt;i=i+2047)
                {
                        int_p[i]=i;
                        //printf("Check\n");
                }
		sleep(8);
		cnt++;
		free(int_p);
		sleep(4);
	}
}
int main(){
	sleep(10);
	printf("Started\n");
	create_delete();
	sleep(2);
	return 0;
}

