


//Jakub
//Project

#include <sys/sem.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <math.h>
#include <pthread.h>

using namespace std;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
 int val; /* value for SETVAL */
 struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
 unsigned short *array; /* array for GETALL, SETALL */
 /* Linux specific part: */
 struct seminfo *__buf; /* buffer for IPC_INFO */
};
#endif

//defining struct
typedef struct WorkerStrt
{
	int *ptrbuffer;
	int numOfReadERRS;
	int mutexID;
	int semaphoreID;
	double sleepamount;
	int nWorkersID;
	int nWorkers;
	int nBuffers;
}WorkerStrt;

//worker function
void* worker(void *arg)
{
	int ValNext;
	int SemaphoresON = 0;

	WorkerStrt *SingleWorker;
	SingleWorker = (WorkerStrt*)arg;

	//intializing buffer that will be accessed next
	ValNext = SingleWorker->nWorkersID;
	

	//semaphores:creating locking 
	struct sembuf lock;
	lock.sem_num = ValNext;
	lock.sem_op = -1;
	lock.sem_flg = 0;

	//creating mutex Locking 
	struct sembuf mutex_lock;
	mutex_lock.sem_num = 0;
	mutex_lock.sem_op = 1;
	mutex_lock.sem_flg = 0;

	//creating mutex unLocking
	struct sembuf mutex_unlock;
	mutex_unlock.sem_num = 0;
	mutex_unlock.sem_op = -1;
	mutex_unlock.sem_flg = 0;

	//semaphores:creating unlocking
	struct sembuf unlock;
	unlock.sem_num = ValNext;
	unlock.sem_op = 1;
	unlock.sem_flg = 0;

	//checking if semaphore used/declared
	if(SingleWorker->semaphoreID >= 0){

		SemaphoresON = 1;

	}
	

	//Looping through all the buffer
	for(int a = 0; a < SingleWorker->nBuffers; a++)
	{
		// current buffer the worker should access
		int CurrBuff = ValNext;

		//looping 3 times, 1,2 are reads, 3 is write
		for(int b = 1; b <= 3; b++)
		{

			//checking for lock error
			if(SemaphoresON == 1){

				if(semop(SingleWorker->semaphoreID, &lock, 1) < 0)
				{

					printf("ERR: Semaphore error, locked.");
					exit(-9);

				}
			}
			//Getting the firstread value from buffer
			int firstRead = SingleWorker->ptrbuffer[ValNext];


			// sleeping as professor said to do so
			usleep(1000000*SingleWorker->sleepamount);

			//if b is not 1, or 2, perform else otherwise write to the buffer
			if(!((b == 3) && (b != 1) && (b != 2)))
			{
				//getting the second read
				int ReadSecond = SingleWorker->ptrbuffer[ValNext];

				//if they dont match then perform a lock
				if(firstRead != ReadSecond)
				{

					//mutex here so they dont overwrite each other
					if(semop(SingleWorker->mutexID, &mutex_lock, 1) < 0)
					{

						//
						printf("ERR: mutex error occurred during locking");
						exit(-10);	

					}

					//prntf statement info about read errors
					printf("Read error has occurred. Worker %d has reported changes from %d to %d ",SingleWorker->nWorkersID, firstRead, ReadSecond);

					//displays bad bits
					printf("Bad bits:");

					//SHowing error of unlocking
					if(semop(SingleWorker->mutexID, &mutex_unlock, 1) < 0)
					{
						//
						printf("ERR: mutex error occurred during unlocking");
						exit(-11);

					}

					//Going throught alll the workers
					for(int l = 0; l < SingleWorker->nWorkers; l++)
					{
						//checking the bit
						int checkingbit = 1 << l;

						//bit changed
						if((checkingbit & firstRead) != (checkingbit & ReadSecond))
						{
							//negative bit
							if(checkingbit & firstRead){

								cout << " -" << l;

							}
							//postive bit
							else if(checkingbit & ReadSecond){

								cout << " +" << l;

							}
						}
					}

					//new line character for visuality
					printf("\n");

					//incrmeenting read errors *IMPORTANT*
					SingleWorker->numOfReadERRS++;
				}
				

			}
			//here it reads aka b = 1, next time around b = 2
			else
			{

				SingleWorker->ptrbuffer[ValNext] =  firstRead + (1 << (SingleWorker->nWorkersID - 1));

				
			}

			//if semaphores are in use then perform unlocking
			if(SemaphoresON == 1){
				if(semop(SingleWorker->semaphoreID, &unlock, 1) < 0)
				{
					//error message if anything goes wrong
					perror("error unlocking semaphore");
					exit(-12);

				}
			}

			//using professors algoirthm to go to the next buffer
			ValNext = (CurrBuff + SingleWorker->nWorkersID) % SingleWorker->nBuffers;

			//locking and unlocking, being set to the next value
			lock.sem_num = ValNext;
			unlock.sem_num = ValNext;
		}
	}

	return NULL;
}



int main(int argc, char** argv){

//declaring variables
int nBuffers = 0;
int nWorkers = 0;
double sleepMin = 1.0;
double sleepMax = 5.0;
int randSeed = 0;
int lock1;

//arguement parsing and checking
if(argc < 3){
	cout << "Not enough arguements, please try again" << endl;
}
else{
	cout << "RaceTest program has Begun" << endl << endl;
	nBuffers = atoi(argv[1]);
	nWorkers = atoi(argv[2]);
}

//buffer error checking
if( nBuffers > 2 && nBuffers < 32){
	if((nBuffers % 2 != 0) && (nBuffers % 3 != 0) && (nBuffers % 5 != 0)){

	}
	else{
		cout << "nBuffers must be a prime number " << endl;
		exit(-1);
	}
}
//buffer error checking
else{
	cout << "ERR: nBuffers must be between 2 < nBuffers < 32" << endl;
	exit(-1);
}

//wroker error checking
if(nWorkers < nBuffers){

}
else{
	cout <<"Workers must be less then nBuffers" << endl;
	exit(-1);
}
//cout << "RaceTest, enter in this order: nBuffers nWorkers sleepMin sleepMax randSeed [-lock OR -nolock]"<< endl;

//sleepmin
if(argc > 3){
	sleepMin = atof(argv[3]);
}

//sleepmax
if(argc > 4){
	sleepMax = atof(argv[4]);
}

//max > min
if(sleepMin > 0.0 || sleepMax > 0.0){
	if(sleepMin < sleepMax){

	}
	else{
		cout <<"sleepMin cannot be bigger than sleepMax" << endl;
		exit(-1);
	}
}
//must be > 0
else{
	cout << "SleepMin and SleepMax must be positive";
}

//seed
if(argc > 5){
		randSeed = atoi(argv[5]);
		cout << "randSeed updated" << endl;
	
}

//-lock error checking
lock1 = 0;

if(argc > 6){
	if(strcmp("-lock", argv[6]) == 0)
	{
		cout << "Lock mode ON" << endl << endl;
		lock1 = 1;
	}
	else{
		cout << "Lock mode OFF" << endl << endl;
	}
}

//seed launching
if(randSeed == 0){
		srand(time(NULL));
	}
	else{
		srand(randSeed);
	}

int semaphoreID = -1;
int mutexID = -1; 

//creating semaphoreArray
unsigned short semaphoreArray[nBuffers];

for(int a = 0; a < nBuffers; a++)
{
	semaphoreArray[a] = 1;
}

//using union struct
union semun semun2; 
union semun semun1;


semun2.val = 1;
semun1.array = semaphoreArray;

//error checking
if(lock1 == 1){
	if((semaphoreID = semget(IPC_PRIVATE, nBuffers, IPC_CREAT | 0600)) < 0)
	{
		printf("ERR: SemaphoreArray has failed.");
		exit(-2);
	}

	if(semctl(semaphoreID, 0, SETALL, semun1) < 0)
	{
		printf("ERR: semctl for SemaphoreArray has failed");
		exit(-2);
	}
}


mutexID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
int val2 = semctl(mutexID, 0, SETVAL, semun2);

//error checking
if(lock1 == 1 || lock1 == 0){

	if( mutexID < 0)
	{
		printf("ERR: mutex has failed.");
		exit(-2);
	}

	if( val2< 0)
	{
		printf("ERR: semctl for mutex has failed");
		exit(-2);
	}
}


//Debugging purposes
printf("nBuffers: %d\n", nBuffers);
printf("nWorkers: %d\n", nWorkers); 
printf("sleepMin: %lf\n", sleepMin);
printf("sleepMax: %lf\n", sleepMax);
printf("randSeed: %d\n", randSeed); 
printf("lock: %d\n\n", lock1);

int arraynBuffers[nBuffers];

for(int a = 0; a < nBuffers; a++)
{
	arraynBuffers[a] = 0;
}


WorkerStrt workerStruct[nWorkers];


for(int a = 0; a < nWorkers; a++){
	workerStruct[a].numOfReadERRS = 0;
	workerStruct[a].mutexID = mutexID;
	workerStruct[a].semaphoreID = semaphoreID;
	//professor said to generate it as  A + ( B - A ) * rand( ) / RANDMAX.
	workerStruct[a].sleepamount = sleepMin + (sleepMax - sleepMin) * rand() / RAND_MAX;
	workerStruct[a].nWorkersID = a + 1;
	workerStruct[a].nBuffers = nBuffers;
	workerStruct[a].ptrbuffer = arraynBuffers;
	workerStruct[a].nWorkers = nWorkers;
}


pthread_t newThreadIDs[nWorkers];

for (int b = 0; b < nWorkers; b++){
	pthread_create(&newThreadIDs[b], NULL, worker, &workerStruct[b]);
}


for(int c = 0; c < nWorkers; c++)
{
	pthread_join(newThreadIDs[c], NULL);
}

printf("\nThreads have been successfully joined together\n");

//intializing variable to show info 
	long int NumOfFinalVals = ((1 << nWorkers) - 1);
	
	printf("Final value is: %d\n", NumOfFinalVals);

	printf("Write errors reporting below.\n\n"); 

	int NumOfWriteErr = 0;

	//write errors for loop
	for(int a = 0; a < nBuffers ; a++)
	{
		//Using XOR to see difference in bits(professors said to do this)
		if(NumOfFinalVals ^ arraynBuffers[a])
		{
			printf("Write error in Buffer %d. The real value contains: %d ", a, arraynBuffers[a]); 
			printf("Bad bits are: ");

			for(int b = 0; b < nWorkers; b++)
			{
				int singlebit = 1 << b;

				if((singlebit & arraynBuffers[a]) == 0)
				{
					//showing bad bits
					printf("%d ", b);

					//increment number ofwrite errors
					NumOfWriteErr++;
				}
			}

			printf("\n");
			//printf("\n");
		}
		
	}

	int NumOfReadErr = 0;

	//read errors for loop
	for(int c = 0; c < nWorkers; c++){
		NumOfReadErr = NumOfReadErr + workerStruct[c].numOfReadERRS;
	}

	printf("\nNumber of read ERRORS: %d",NumOfReadErr); 
	printf("\nNumber of write ERRORS: %d \n", NumOfWriteErr);

	//free memory
	semctl(mutexID, 0, IPC_RMID, semun2);
	semctl(semaphoreID, 0, IPC_RMID, semun1);
	
	return 2;
}




