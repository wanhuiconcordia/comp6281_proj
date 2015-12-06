#include "tools.h"
#include <time.h>
#include <string.h>

void setRandSeed(){
    time_t t;
    srand((unsigned) (time(&t) + &t));
}

int isValidInt(char* str){
    char* p = str;
    while(*p){
        if(*p < '0' || *p > '9'){
            if(*p == '-'){
                if(p != str){
                    return 0;
                }
            }else{
                return 0;
            }
        }
        p++;
    }
    return 1;
}

int isValidFloat(char* str){
    char* p = str;
    char * strTail = str + strlen(str) - 1;
    int dotOccurance = 0;
    while(*p){
        if(*p < '0' || *p > '9'){
            if(*p == '-'){
                if(p != str){
                    return 0;
                }
            }else if(*p == '.'){
                dotOccurance++;
                if(dotOccurance > 1){
                    return 0;
                }

                if(p == strTail){
                    return 0;
                }
            }else{
                return 0;
            }
        }
        p++;
    }
    return 1;
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
