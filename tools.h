#ifndef TOOLS_H
#define TOOLS_H
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#define TRUE 1
#define FALSE 0
#define MAX_EVENT_SIZE 10000000
#define SLEEP_INTERVAL 100
void setRandSeed();

int isValidInt(char* str);
int isValidFloat(char* str);
int unlockSem(int semid);
int lockSem(int semid);
#endif // TOOLS_H
