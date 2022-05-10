
//Creating a simulation of a car wash 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>			/* NULL */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#define MAX_CARS 1000
#define MAX_WASH_POS 5
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// Definition of the variables
clock_t t;
double time_taken=0;
int totalSlots;
int washingPositionsSlots;
int* washingFacility=NULL;
int carLeves=0;
pthread_t threads[MAX_CARS];
void* idTread[MAX_CARS];
pthread_cond_t leaveSpace = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexin = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexout = PTHREAD_MUTEX_INITIALIZER;

// Definition of the signals
void sigquit(){
	printf("total number of vehicles washed: %d",carLeves);
	printf(" \nAverage waiting time in queue of cars wasshed %f\n",time_taken/carLeves);
	printf("MY DADDY HAS KILLED ME\n");
	exit(0);
}

void hadle_sigint(){
	printf("\ntotal number of vehicles washed: %d\n",carLeves);
	printf("Average waiting time in queue of cars wasshed %f\n",time_taken/carLeves);
	signal(SIGINT, hadle_sigint);	
	printf("\t Washing facility is closing...\n");
	for(int i=0;i<MAX_CARS;i++)
	{
		if(idTread[i]!=NULL)
			printf("Car (trhead) %p exited from washing facility\n",idTread[i]);	
	}
	printf("\t Washing facility is closed...\n");
	exit(0);
}

float ran_expo(float lambda){
    float u;
    u = rand() / (RAND_MAX + 1.0);
    return  -log(10) / lambda;
}
void *assignSpace(void *threadid){
	// When thread goes in, it checks if the washing facility is full.
	pthread_mutex_lock(&mutexin);
	if(totalSlots < 0){
		printf("Washing facility full. Thread %p will wait\n",threadid);
		pthread_cond_wait(&leaveSpace, &mutexin);
	}
	totalSlots--;
	int j, ib,jb,k, counter = 0;
	// The car must look for a place in p floor.. then p+1 etc..
	for( j = 0; j < washingPositionsSlots; j++){
		t=clock();
		if(washingFacility[j] == 0){
			
			idTread[j]=threadid;
		    	printf("Position %d is free. Car (thread) %p is taking it...",j,threadid);
		    	washingFacility[j]=1;
		    	printf("TAKEN \n");
		    	jb=j;
		    	for(k = 0 ; k < washingPositionsSlots; k++)
		    		if(washingFacility[k] == 0)
		    			  counter++;

		    	j=washingPositionsSlots; // This forces the program to go out of the loop
			t=clock()-t;
			time_taken+= ((double)t)/CLOCKS_PER_SEC;
				
	      }

	}
	pthread_mutex_unlock(&mutexin);
	pthread_mutex_lock(&mutexout);
	srand(time(NULL));
	sleep(ran_expo(-0.7));
	counter=0;
	washingFacility[jb]=0;
	
	for(k = 0 ; k < washingPositionsSlots; k++)
		if(washingFacility[k] == 0)
		    	counter++;

	totalSlots++;
	carLeves++;
	printf("Car (thread) %p leaves washing facility, %d vacancies\n",threadid, totalSlots);

	// As soon as the thread finishes a signal is emitted to the threads waiting.
	pthread_cond_signal(&leaveSpace);
	pthread_mutex_unlock(&mutexout);
	pthread_exit(0);
}

void main (int argc, char *argv[]){
	int pid;
	int rc;
	if((pid=fork())<0){ /* Get child process */
		printf("fork");
		exit(1);
	}

	if(pid==0){ /* Child */
		signal(SIGINT, hadle_sigint);
		signal(SIGQUIT,sigquit);
		int N;
		while(N>MAX_WASH_POS){
			printf("Enter a few washing positions:\n");
			scanf("%d", &N);
		}
		printf("Assigning washingFacility spaces per level...");
		washingPositionsSlots = N;
		printf("DONE\n");
		totalSlots = washingPositionsSlots;
		printf("Initializing washing facility spots...\n");
		washingFacility = malloc(washingPositionsSlots*sizeof(int));
		if(washingFacility == NULL){
			perror("out of memory\n");
			exit(1);
		}
		int numbOfVehicle=1;
		while (numbOfVehicle<12){
			sleep(ran_expo(-1.5));
			printf("Creating thread for car %d\n", numbOfVehicle);
			// Assign a car to a position if possible in assignSpace
			rc = pthread_create(&threads[numbOfVehicle], NULL, assignSpace, (void *)(intptr_t)numbOfVehicle);

			if (rc){
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
			numbOfVehicle++;
				
		}

		for(int i=1;i<12;i++)
			pthread_join(threads[i],NULL);
		printf("total number of vehicles washed: %d\n",carLeves);
		//printf(" \nrrrr %f\n",time_taken);
		printf("Average waiting time in queue of cars wasshed %f\n",time_taken/carLeves);
	}
	else{ /* Parent */
		sleep(30); /* Pause for 30 second */
		kill(pid,SIGINT);
		kill(pid,SIGQUIT);	
	}
}







