#include <mpi.h>
#include <signal.h>
#include <unistd.h>
#include "even.h"
#include "tools.h"
#include "event.h"
#include "menu.h"
//global variables for parent process
extern int isChildProcessRunning;
extern int isQueryDataReady;

//global variables for both processes
extern int keepWorking;


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


void runEven(int pid, int semid, int shmid, int argc, char **argv){
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
                        printf("\nType enter to contine...\n");
                        getchar();
                        getchar();
                    }
                }else{
                    lockSem(semid);
                    localEventsCount = *((int *)shareData);
                    //printf("rank:%d get %d events from child process\n", rank, localEventsCount);
                    if(localEventsCount > 0){
                        memcpy(localEvents, shareData + sizeof(int), localEventsCount * sizeof(Event));
                        //                            printf("rank:%d, events are:\n", rank);
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
                    //                        for(int i = 0; i < size; i++){
                    //                            printf("rank:%d\tdisp[%d]:%d\tsendCounts[%d]:%d\n", rank, i, displacements[i], i, sendCounts[i]);
                    //                        }

                    //                        for(int i = 0; i < size; i++){
                    //                            sendCounts[i] = 1;
                    //                        }
                    //                        for(int i = 0; i < size; i++){
                    //                            MPI_Scatterv(localEvents, sendCounts, displacements, EventType, bucketEvents[i], MAX_EVENT_SIZE, EventType, i, MPI_COMM_WORLD);
                    //                        }

                    MPI_Request sendRequests[size];

                    for(int i = 0; i < size; i++){
                        MPI_Isend (localEvents + displacements[i], sendCounts[i], EventType, i, 1, MPI_COMM_WORLD, sendRequests + i);
                    }

                    MPI_Status status;
                    for(int i = 0; i < size; i++){
                        MPI_Recv (bucketEvents[i], MAX_EVENT_SIZE, EventType, i, 1, MPI_COMM_WORLD, &status);
                        MPI_Get_count (&status, EventType, &bucketEventCount[i]);
                    }

                    //printf("rank %d After scatter events\n", rank);
                    for(int i = 0; i < size; i++){
                        MPI_Gather(&sendCounts[i], 1, MPI_INT, bucketEventCount, 1, MPI_INT, i, MPI_COMM_WORLD);
                    }

                    //sleep(rank + 1);
                    //printf("rank:%d bucketEvents\n", rank);
                    //                        for(int i = 0; i < size; i++){
                    //                            printf("bucketEventCount[%d]:%d\n", i, bucketEventCount[i]);
                    //                            printEvent(bucketEvents[i], bucketEventCount[i]);
                    //                            usleep(500);
                    //                        }

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

                        printf("Total sorted event count:%d\n", finalEventCount);
                        if(finalEventCount > 0){
                            if(finalEventCount < 21){
                                printf("Sorted events:\n");
                                printEvent(finalEvents, finalEventCount);
                            }else{
                                printf("First 10 events:\n");
                                printEvent(finalEvents, 10);
                                printf("Last 10 events:\n");
                                printEvent(finalEvents + finalEventCount - 10, 10);
                            }
                        }
                        printf("\nType enter to contine...\n");
                        getchar();
                        getchar();
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

    semctl(semid, 0, IPC_RMID, NULL);
    shmdt(shareData);
    shmctl(shmid, IPC_RMID, NULL);
    MPI_Finalize();
    //printf("Parent process finished\n");
}
