#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "segun.h"
#include "event.h"
#include "odd.h"
#include "even.h"

//global variables for parent process
int isChildProcessRunning = FALSE;
int isQueryDataReady = FALSE;

//global variables for child process
int isPendingReadRequired = FALSE;
int isQueryRequired = FALSE;

//global variables for both processes
int keepWorking = TRUE;

int main(int argc, char **argv)
{
    if(argc != 2){
        printf("ONE parameter for read_max[100, 100000] must be provided.\n");
        return 0;
    }

    int read_max = -1;
    if(isValidInt(argv[1])){
        read_max = atoi(argv[1]);
    }

    if(read_max < 100 || read_max > 100000){
        printf("read_max range is [100, 100000] \n");
        return 0;
    }

    int semid, shmid;
    //Create semaphore
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1){
        printf("Failed to create sem\n");
        return -1;
    }

    //Initialize val of semaphore
    union semun arg, dummy;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1){
        printf("Failed to set sem val\n");
        semctl(semid, 0, IPC_RMID, dummy);
        return -1;
    }

    //Create sharedmemory
    shmid = shmget(IPC_PRIVATE, MAX_EVENT_SIZE * sizeof(Event), IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1){
        semctl(semid, 0, IPC_RMID, dummy);
        return -1;
    }

    //printf("semid:%d, shmid:%d\n", semid, shmid);
    setRandSeed();

    int pid = fork();

    if(pid == 0){   //child process
        runOdd(read_max, semid, shmid);
    }else if(pid > 0){      //parent process
        runEven(pid, semid, shmid, argc, argv);
    }else{  //error fork
        printf("Failed to fork()\n");
        semctl(semid, 0, IPC_RMID, dummy);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
