#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>
#include <mpi.h>

#include "event.h"
#include "segun.h"
#include "tools.h"

#define clear() printf("\033[H\033[J")



#define UNKNOWN_QUERY 0
#define COMPANY_SALES 1
#define SALES_BY_DATE 2
#define DELETE_BY_DATE 3
#define USER_EXIT 4
#define PARENT_PROCESS_READY 5

#define SLEEP_INTERVAL 100


//global variables for parent process
int isChildProcessRunning = FALSE;
int isQueryDataReady = FALSE;

//global variables for child process
int isPendingReadRequired = FALSE;
int isQueryRequired = FALSE;

//global variables for both processes
int keepWorking = TRUE;
int semid = 0;
int shmid = 0;

void parentSignalHandler(int sig){
    switch(sig){
    case SIGUSR1:	//child process finished
        isChildProcessRunning = FALSE;
        keepWorking = FALSE;
        break;
    case SIGUSR2:	//child process prepared the query data
        isQueryDataReady = TRUE;
        break;
    default:
        printf("unknown signal:%d\n", sig);
        break;
    }
}

void childSignalHandler(int sig){
    switch(sig){
    case SIGALRM:	//time round comes to read more date from Database(file)
        //printf("process:%d get a SIGALRM signal and will set isPendingReadRequired to TRUE then start the timer again\n", getpid());
        isPendingReadRequired = TRUE;
        alarm(3);	//random(3-5)
        break;
    case SIGUSR1:	//parent tell me to terminate
        //printf("process:%d get a SIGUSR1 signal and will terminate this process by setting the keepWorking to FALSE\n", getpid());
        keepWorking = FALSE;
        break;
    case SIGUSR2:	//parent tell me to do query
        //printf("process:%d get a SIGUSR2 signal and set isQueryRequired to TRUE\n", getpid());
        isQueryRequired = TRUE;
        break;
    default:
        printf("unknown signal:%d\n", sig);
        break;
    }
}


int unlockSem(int semid){
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    /* Add, subtract, or wait for 0 */
    sop.sem_flg = 0;
    int iRet = semop(semid, &sop, 1);
    return iRet;
}

int lockSem(int semid){
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = -1;
    /* Add, subtract, or wait for 0 */
    sop.sem_flg = 0;
    int iRet = semop(semid, &sop, 1);
    return iRet;
}

int inputNumber(){
    char selection[100];
    while(TRUE){
        scanf("%s", selection);
        if(isValidInt(selection)){
            return atoi(selection);
        }else{
            printf("\t\tPlease input a integer:");
        }
    }

}

void dateMenu(Date* date1, Date* date2){
    while(TRUE){
        printf("\t\tEnter Starting Year:");
        fflush(stdout);
        date1->year = inputNumber();
        printf("\t\tEnter Starting Month:");
        fflush(stdout);
        date1->month = inputNumber();
        printf("\t\tEnter Starting Day:");
        fflush(stdout);
        date1->day = inputNumber();
        if(isValidDate(*date1)){
            break;
        }else{
            printf("\t\tThis is not a valid date. Try again.\n");
            fflush(stdout);
        }
    }

    while(TRUE){
        printf("\t\tEnter Ending Year:");
        fflush(stdout);
        date2->year = inputNumber();
        printf("\t\tEnter Ending Month:");
        fflush(stdout);
        date2->month = inputNumber();
        printf("\t\tEnter Ending Day:");
        fflush(stdout);
        date2->day = inputNumber();
        if(isValidDate(*date2)){
            if(dateCmp(date1, date2) > 0){
                printf("\t\tEnding date should be later than Starting date. Try again.\n");
            }else{
                break;
            }
        }else{
            printf("\t\tThis is not a valid date. Try again.\n");
            fflush(stdout);
        }
    }
}

void mainMenu(Query* pQuery){
    memset(pQuery, 0, sizeof(Query));
    while(TRUE){
//        clear();
        printf("\n+++++++++++++++++++++++\n");
        printf("\t1. Company Sales\n");
        printf("\t2. Sales by Date\n");
        printf("\t3. Delete by Date\n");
        printf("\t4. Exit\n");
        printf("+++++++++++++++++++++++\n");
        printf("Selection:");
        fflush(stdout);
        char selection[100];
        scanf("%s", selection);
        if(strcmp(selection, "1") == 0){
            pQuery->type = COMPANY_SALES;
            break;
        }else if(strcmp(selection, "2") == 0){
            pQuery->type = SALES_BY_DATE;
            dateMenu(&(pQuery->date1), &(pQuery->date2));
            break;
        }else if(strcmp(selection, "3") ==0){
            pQuery->type = DELETE_BY_DATE;
            dateMenu(&(pQuery->date1), &(pQuery->date2));
            break;
        }else if(strcmp(selection, "4") == 0){
            pQuery->type = USER_EXIT;
            break;
        }else if(strcmp(selection, "\n") == 0){
        }else{
            printf("\tWrong selection, try again!\n");
            printf("\tType enter to contine...\n");
            fflush(stdout);
            getchar();
            getchar();
        }
    }
}

int main(int argc, char **argv)
{
    if(argc != 2){
        printf("ONE parameter for read_max[100, 100000] must be provided.\n");
        return 0;
    }

    if(!isValidInt(argv[1])){
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
        int read_max = 1000;
        if(argc > 0){
            int tmpNum = atoi(argv[1]);
            if(tmpNum >= 100 && tmpNum <= 100000){
                read_max = tmpNum;
            }
        }

//        int eventCountPerTimer = rand() % (read_max - 10) + 10;
        int eventCountPerTimer = 10;
        int parentPid = getppid();
        //printf("parent pid: %d, child pid:%d\n", parentPid, getpid());
        void *shareData = shmat(shmid, NULL, 0);
        signal(SIGUSR1, childSignalHandler);
        signal(SIGUSR2, childSignalHandler);
        signal(SIGALRM, childSignalHandler);
        Event* rawEvents = malloc(MAX_EVENT_SIZE * sizeof(Event));
        int rawEventCount = 0;
        Event* queriedEvents = malloc(MAX_EVENT_SIZE * sizeof(Event));
        int queriedEventCount = 0;
//        int cookedEventCountToParent = 0;
        Query query;
        char fileName[10];
        FILE* file = NULL;
        while(keepWorking){
            if(isQueryRequired){
                isQueryRequired = FALSE;
                lockSem(semid);
                memcpy(&query, shareData, sizeof(Query));
                unlockSem(semid);
                if(query.type == PARENT_PROCESS_READY){
                    strcpy(fileName, query.dataFileName);
                    file = fopen(fileName, "r");
                    if(file){
                        isPendingReadRequired = TRUE;
                        alarm(3);
                    }else{
                        printf("Cannot open file\n");
                        keepWorking = FALSE;
                    }
                }else if(query.type == COMPANY_SALES){
                    sortEventsByCompanyName(rawEvents, rawEventCount);
                    lockSem(semid);
                    memcpy(shareData, &rawEventCount, sizeof(int));
                    memcpy(shareData + sizeof(int), rawEvents, rawEventCount * sizeof(Event));
                    //                    printEvent(shareData + sizeof(int), rawEventCount);
                    unlockSem(semid);
                    kill(parentPid, SIGUSR2);
                }else if(query.type == SALES_BY_DATE){
                    queriedEventCount = 0;

                    int minDays = dayFromEpoch(query.date1);
                    int maxDays = dayFromEpoch(query.date2);
                    int thisDays;
                    for(int i = 0; i < rawEventCount; i++){
                        thisDays = dayFromEpoch(rawEvents[i].date);
                        if(thisDays >= minDays && thisDays <= maxDays){
                            queriedEvents[queriedEventCount++] = rawEvents[i];
                        }
                    }
                    //                    usleep(rand() % 5 + 1);
                    //                    printf("**********Before date sorting:\n");
                    //                    printEvent(queriedEvents, queriedEventCount);
                    //                    printf("**********After date sorting:\n");
                    sortEventsByDate(queriedEvents, queriedEventCount);

                    //                    printEvent(queriedEvents, queriedEventCount);
                    lockSem(semid);
                    memcpy(shareData, &queriedEventCount, sizeof(int));
                    memcpy(shareData + sizeof(int), queriedEvents, queriedEventCount * sizeof(Event));
                    unlockSem(semid);
                    kill(parentPid, SIGUSR2);
                }else{//DELETE_BY_DATE
                    //delete memory events and put the count in shared memory
                    int deletedEventCount = 0;
                    int minDays = dayFromEpoch(query.date1);
                    int maxDays = dayFromEpoch(query.date2);
                    int thisDays;
                    for(int i = 0; i < rawEventCount;){
                        thisDays = dayFromEpoch(rawEvents[i].date);
                        if(thisDays >= minDays && thisDays <= maxDays){
                            rawEvents[i] = rawEvents[rawEventCount - 1];
                            deletedEventCount++;
                            rawEventCount--;
                        }else{
                            i++;
                        }
                    }
                    lockSem(semid);
                    *((int*)shareData) = deletedEventCount;
                    //printf("child process deleted %d events\n", deletedEventCount);
                    unlockSem(semid);
                    kill(parentPid, SIGUSR2);
                }
            }

            if(isPendingReadRequired){
                readEvent(file, eventCountPerTimer, rawEvents, &rawEventCount);
                //                printEvent(rawEvents, rawEventCount);
                isPendingReadRequired = FALSE;
            }
            usleep(SLEEP_INTERVAL);
        }

        //printf("child process finished\n");
        free(queriedEvents);
        free(rawEvents);

        kill(parentPid, SIGUSR1);	//tell parent process that I am ready to terminate and will not touch the shared resource
        shmdt(shareData);
    }else if(pid > 0){      //parent process
        int size, rank;
        int childPid = pid;
        //printf("parent pid: %d, child pid:%d\n", getpid(), childPid);
        void *shareData = shmat(shmid, NULL, 0);
        isChildProcessRunning = TRUE;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        if(size >= 1 && size <=5){
            signal(SIGUSR1, parentSignalHandler);
            signal(SIGUSR2, parentSignalHandler);

            MPI_Datatype EventType;
            initEventType(&EventType);

            MPI_Datatype QueryType;
            initQueryType(&QueryType);

            Event* localEvents = malloc(MAX_EVENT_SIZE * sizeof(Event));                  //Save events from child process
            int localEventsCount = 0;

            Event* finalEvents = malloc(MAX_EVENT_SIZE * size * sizeof(Event));           //Save the final events(rank 0 use it)
            int finalEventCount = 0;

            Event* bucketEvents[size];           //Save events from peer MPI nodes
            for(int i = 0; i < size; i++){
                bucketEvents[i] = malloc(MAX_EVENT_SIZE * sizeof(Event));
            }
            int bucketEventCount[size];

            Event* mergedBucketEvents = malloc(MAX_EVENT_SIZE * size * sizeof(Event));    //Save the merged bucket events
            int mergedBucketEventCount = 0;

            int localDelCount = 0;
            int bucketDelCount[size];

            int sendCounts[size];
            int displacements[size];


            Query query;
            query.type = PARENT_PROCESS_READY;
            sprintf(query.dataFileName, "data%d.txt", rank);
            lockSem(semid);
            memcpy(shareData, &query, sizeof(Query));
            unlockSem(semid);
            //printf("rank:%d, sent SIGUSR2 signal to its child process:%d\n", rank, childPid);
            kill(childPid, SIGUSR2);

            while(keepWorking){
                //printf("rank:%d is running...\n", rank);
                if(rank == 0){
                    mainMenu(&query);
                    if(query.type == USER_EXIT){
                        keepWorking = FALSE;
                    }
                }
                //bcast the query
                MPI_Bcast(&query, 1, QueryType, 0, MPI_COMM_WORLD);
//                printf("rank:%d\t", rank);
//                printQuery(&query, 1);
                if(query.type == USER_EXIT){
                    keepWorking = FALSE;
                }else{
                    lockSem(semid);
                    memcpy(shareData, &query, sizeof(Query));
                    unlockSem(semid);
                    isQueryDataReady = FALSE;
                    kill(childPid, SIGUSR2);
                    while (!isQueryDataReady) {
                        usleep(SLEEP_INTERVAL);
                    }
                    if(query.type == DELETE_BY_DATE){
                        lockSem(semid);
                        localDelCount = *((int *)shareData);
                        //printf("parent process received deleted %d events\n", localDelCount);
                        unlockSem(semid);
                        MPI_Gather(&localDelCount, 1, MPI_INT, bucketDelCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        if(rank == 0){
                            int finalDeletedCount = 0;
                            for(int i = 0; i < size; i++){
                                finalDeletedCount += bucketDelCount[i];
                            }
                            printf("%d events deleted\n", finalDeletedCount);
                        }
                    }else{
                        lockSem(semid);
                        localEventsCount = *((int *)shareData);
                        printf("rank:%d get %d events from child process\n", rank, localEventsCount);
                        if(localEventsCount > 0){
                            memcpy(localEvents, shareData + sizeof(int), localEventsCount * sizeof(Event));
                            printf("rank:%d, events are:\n", rank);
                            //                            printEvent(shareData + sizeof(int), localEventsCount);
                        }
                        unlockSem(semid);
                        if(query.type == COMPANY_SALES){
                            //sortEventsByCompanyName(localEvents, localEventsCount);

                            calcDisplacementByCompanyName(localEvents, localEventsCount, sendCounts, displacements, size);

                        }else{
                            //sortEventsByDate(localEvents, localEventsCount);
                            calcDisplacementByDate(localEvents, localEventsCount, sendCounts, displacements, size, query.date1, query.date2);
                        }
                        for(int i = 0; i < size; i++){
                            printf("rank:%d\tdisp[%d]:%d\tsendCounts[%d]:%d\n", rank, i, displacements[i], i, sendCounts[i]);
                        }

                        //                        for(int i = 0; i < size; i++){
                        //                            sendCounts[i] = 1;
                        //                        }
                        for(int i = 0; i < size; i++){
                            MPI_Scatterv(localEvents, sendCounts, displacements, EventType, bucketEvents[i], MAX_EVENT_SIZE, EventType, i, MPI_COMM_WORLD);
                        }


                        printf("rank %d After scatter events\n", rank);
                        for(int i = 0; i < size; i++){
                            MPI_Gather(&sendCounts[i], 1, MPI_INT, bucketEventCount, 1, MPI_INT, i, MPI_COMM_WORLD);
                        }

                        sleep(rank + 1);
                        printf("rank:%d bucketEvents\n", rank);
                        for(int i = 0; i < size; i++){
                            printf("bucketEventCount[%d]:%d\n", i, bucketEventCount[i]);
                            printEvent(bucketEvents[i], bucketEventCount[i]);
                            usleep(500);
                        }

                        if(size == 1){
                            memcpy(mergedBucketEvents, bucketEvents[0], mergedBucketEventCount * sizeof(Event));
                        }else{
                            if(query.type == COMPANY_SALES){
                                mergeEvents(mergedBucketEvents, &mergedBucketEventCount, bucketEvents, bucketEventCount, size, 1/*by company_name*/);
                            }else {
                                mergeEvents(mergedBucketEvents, &mergedBucketEventCount, bucketEvents, bucketEventCount, size, 0/*by date*/);
                            }
                        }

//                        sleep(1);
//                        if(rank == 1){
//                            printf("mergedBucketEventCount:%d\n", mergedBucketEventCount);
//                            printEvent(mergedBucketEvents, mergedBucketEventCount);
//                        }
//                        sleep(1);


                        if(rank == 0){
                            finalEventCount = mergedBucketEventCount;
                            memcpy(finalEvents, mergedBucketEvents, mergedBucketEventCount * sizeof(Event));
                            int receivedEventCount = 0;
                            MPI_Status status;
                            for(int i = 1; i < size; i++){
                                MPI_Recv (finalEvents + finalEventCount, MAX_EVENT_SIZE * size, EventType, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
                                MPI_Get_count (&status, EventType, &receivedEventCount);
                                finalEventCount += receivedEventCount;
                            }
                            sleep(1);
                            printf("mergedBucketEventCount:%d\n", finalEventCount);
                            printEvent(finalEvents, finalEventCount);
                            sleep(1);
                        }else{
                            MPI_Send (mergedBucketEvents, mergedBucketEventCount, EventType, 0, 1, MPI_COMM_WORLD);
                        }
                    }
                }
            }

            kill(childPid, SIGUSR1);
            while(isChildProcessRunning){
                usleep(SLEEP_INTERVAL); //waiting for child process to finish its job
            }
            free(localEvents);
            free(finalEvents);
            free(mergedBucketEvents);
            for(int i = 0; i < size; i++){
                free(bucketEvents[i]);
            }

            MPI_Type_free(&EventType);
            MPI_Type_free(&QueryType);
        }else{
            kill(childPid, SIGUSR1);
            while(isChildProcessRunning){
                usleep(SLEEP_INTERVAL); //waiting for child process to finish its job
            }
        }

        semctl(semid, 0, IPC_RMID, dummy);
        shmdt(shareData);
        shmctl(shmid, IPC_RMID, NULL);
        MPI_Finalize();
        //printf("Parent process finished\n");
    }else{  //error fork
        printf("Failed to fork()\n");
        semctl(semid, 0, IPC_RMID, dummy);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
