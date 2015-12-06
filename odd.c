#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#include <mpi.h>
#include "odd.h"
#include "tools.h"
#include "event.h"

//global variables for child process
extern int isPendingReadRequired;
extern int isQueryRequired;

//global variables for both processes
extern int keepWorking;

void childSignalHandler(int sig){
    switch(sig){
    case SIGALRM:	//time round comes to read more date from Database(file)
        //printf("process:%d get a SIGALRM signal and will set isPendingReadRequired to TRUE then start the timer again\n", getpid());
        isPendingReadRequired = TRUE;
        alarm(random() % 4 + 2);
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


void runOdd(int read_max, int semid, int shmid){
    int eventCountPerTimer = rand() % (read_max - 10) + 10;

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
            if(rawEventCount < MAX_EVENT_SIZE){
                readEvent(file, eventCountPerTimer, rawEvents, &rawEventCount);
                //                printEvent(rawEvents, rawEventCount);
            }
            isPendingReadRequired = FALSE;
        }
        usleep(SLEEP_INTERVAL);
    }

    //printf("child process finished\n");
    if(file){
        fclose(file);
    }
    free(queriedEvents);
    free(rawEvents);

    kill(parentPid, SIGUSR1);	//tell parent process that I am ready to terminate and will not touch the shared resource
    shmdt(shareData);
}
