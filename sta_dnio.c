#include <pthread.h>		//Create POSIX threads.
#include <time.h>			//Wait for a random time.
#include <unistd.h>			//Thread calls sleep for specified number of seconds.
#include <semaphore.h>		//To create semaphores
#include <stdlib.h>			
#include <stdio.h>			//Input Output

pthread_t *Students;		//N threads running as Students.
pthread_t TA;				//Separate Thread for TA.

int ChairsCount = 0;
int CurrentIndex = 0;
int TotalChairs = 0;
int number_of_students;		//a variable taken from the user to create student threads.	Default is 5 student threads.


//Declaration of Semaphores and Mutex Lock.
sem_t TA_Sleep;
sem_t Student_Sem;
sem_t ChairsSem[3];
pthread_mutex_t ChairAccess;

//Declared Functions
void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	int id;
	srand(time(NULL));

	//Initializing Mutex Lock and Semaphores.
	sem_init(&TA_Sleep, 0, 0);
	sem_init(&Student_Sem, 0, 0);
	for(id = 0; id < 3; ++id)			//Chairs array of 3 semaphores.
		sem_init(&ChairsSem[id], 0, 0);

	pthread_mutex_init(&ChairAccess, NULL);
	
	if(argc<2)
	{
		printf("Number of Students not specified. Using default (5) students.\n");
		number_of_students = 5;
	}
	else
	{
        number_of_students = atoi(argv[1]);
		printf("Number of Students specified. Creating %d threads.\n", number_of_students);
	}
		
	//Allocate memory for Students
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	//Creating TA thread and N Student threads.
	pthread_create(&TA, NULL, TA_Activity, NULL);	
	for(id = 0; id < number_of_students; id++)
		pthread_create(&Students[id], NULL, Student_Activity,(void*) (long)id);

	//Waiting for TA thread and N Student threads.
	pthread_join(TA, NULL);
	for(id = 0; id < number_of_students; id++)
		pthread_join(Students[id], NULL);

	//Free allocated memory
	free(Students); 
	return 0;
}

void *TA_Activity()
{
	while(1)
	{

		while(1)
		{
			// lock
			pthread_mutex_lock(&ChairAccess);
			if(ChairsCount == 0) 
			{
				//if chairs are empty, break the loop.
				pthread_mutex_unlock(&ChairAccess);
				break;
			}
			//TA gets next student on chair.
			sem_post(&ChairsSem[CurrentIndex]);
			ChairsCount--;
			printf("Student left his/her chair. Remaining Chairs %d\n", 3 - ChairsCount);
			CurrentIndex = (CurrentIndex + 1) % 3;


			pthread_mutex_unlock(&ChairAccess);
			// unlock
			sleep(5);
			sem_post(&Student_Sem);
			usleep(1000);
		}
	}
}

void *Student_Activity(void *threadID) 
{
	int ProgrammingTime;

	while(1)
	{

		printf("Student #%ld is going to TA room.\n", (long)threadID);
		ProgrammingTime = rand() % 5;

		pthread_mutex_lock(&ChairAccess);
		int count = ChairsCount;
		pthread_mutex_unlock(&ChairAccess);

		if(count < 3)		//Student tried to sit on a chair.
		{
			if(count == 0){	//If student sits on first empty chair, wake up the TA.
				sem_post(&TA_Sleep);
                printf("Student #%ld is awaking the TA. \n", (long)threadID);
                printf("Student #%ld is getting inside TA room. \n", (long)threadID);
                printf("TA is sleeping\n");
		        sem_wait(&TA_Sleep);		//TA is currently sleeping.
            }
			else{
                printf("%d \n",count);
           		sleep(ProgrammingTime);		//Sleep for a random time period.

            }
			// lock
			pthread_mutex_lock(&ChairAccess);
			int index = (CurrentIndex + ChairsCount) % 3;
			ChairsCount++;
			printf("Student #%ld sat on chair.Chairs Remaining: %d\n", (long)threadID ,3 - ChairsCount);
			
            pthread_mutex_unlock(&ChairAccess);
			// unlock
			sem_wait(&ChairsSem[index]);		//Student leaves his/her chair.
			printf("\t TA is teaching student %ld. \n", (long)threadID);
			sem_wait(&Student_Sem);		//Student waits to go next.
			printf("Student %ld left TA room.\n",(long)threadID);
            TotalChairs += 1;
            if(TotalChairs == number_of_students){
                break;
            }
		}
		else 
			printf("There is no available chairs to student #%ld .The student will return later. \n", (long)threadID);
			//If student didn't find any chair to sit on.
	}
}
