#include "event.h"

#define MAX_LENGTH_PER_LINE 256
void readEvent(FILE *pFile, int readEventCount, Event* events, int* pSize){
    int i = 0;
    char line[MAX_LENGTH_PER_LINE];

    while (fgets(line, sizeof(line), pFile)) {
        parseEvent(events + *pSize, line);
        (*pSize)++;
        if(++i >= readEventCount){
            break;
        }
    }
}

void parseEvent(Event* pEvent, char* eventStr){
    int splitIndex = 0, splitPositions[7];
    splitPositions[0] = 0;
    int strLen = strlen(eventStr);
    for(int i = 0; i < strLen; i++){
        if(eventStr[i] == '|'
                || eventStr[i] == '/'){
            splitPositions[++splitIndex] = i + 1;
            eventStr[i] = 0;
        }
    }
    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->sales_id = atoi(&eventStr[splitPositions[0]]);
    }else{
        printf("Wrong sales_id:%s\n", &eventStr[splitPositions[0]]);
    }

    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->date.year = atoi(&eventStr[splitPositions[1]]);
    }else{
        printf("Wrong year:%s\n", &eventStr[splitPositions[1]]);
    }

    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->date.month = atoi(&eventStr[splitPositions[2]]);
    }else{
        printf("Wrong month:%s\n", &eventStr[splitPositions[2]]);
    }

    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->date.day = atoi(&eventStr[splitPositions[3]]);
    }else{
        printf("Wrong day:%s\n", &eventStr[splitPositions[3]]);
    }

    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->company_id = atoi(&eventStr[splitPositions[4]]);
    }else{
        printf("Wrong company_id:%s\n", &eventStr[splitPositions[4]]);
    }

    strcpy(pEvent->company_name, &eventStr[splitPositions[5]]);

    if(isValidInt(&eventStr[splitPositions[0]])){
        pEvent->sales_total = atof(&eventStr[splitPositions[6]]);
    }else{
        printf("Wrong sales_total:%s\n", &eventStr[splitPositions[6]]);
    }
}

void printEvent(Event* pEvent, int size){
    for(int i = 0; i < size; i++){
        printf("%8d\t%4d-%02d-%02d\t%8d\t%20s%8.2f\n", (pEvent + i)->sales_id, (pEvent + i)->date.year, (pEvent + i)->date.month, (pEvent + i)->date.day, (pEvent + i)->company_id, (pEvent + i)->company_name, (pEvent + i)->sales_total);
    }
}

void initQueryType(MPI_Datatype *pQueryType){
    MPI_Datatype type[3] = {MPI_INT, MPI_SHORT, MPI_CHAR};
    int blocklen[3] = {1, 6, 10};
    Query query;
    MPI_Aint disp[3];
    disp[0] = (void*) &query.type - (void*) &query;
    disp[1] = (void*) &query.date1 - (void*) &query;
    disp[2] = (void*) &query.dataFileName - (void*) &query;
    MPI_Type_create_struct(3, blocklen, disp, type, pQueryType);
    MPI_Type_commit(pQueryType);
}

void initEventType(MPI_Datatype *pEventType){
    MPI_Datatype type[5] = {MPI_INT, MPI_SHORT, MPI_INT, MPI_CHAR, MPI_FLOAT};
    int blocklen[5] = {1, 6, 1, 40, 1};
    Event event;
    MPI_Aint disp[5];
    disp[0] = (void*) &event.sales_id - (void*) &event;
    disp[1] = (void*) &event.date - (void*) &event;
    disp[2] = (void*) &event.company_id - (void*) &event;
    disp[3] = (void*) &event.company_name - (void*) &event;
    disp[4] = (void*) &event.sales_total - (void*) &event;
    MPI_Type_create_struct(5, blocklen, disp, type, pEventType);
    MPI_Type_commit(pEventType);
}

void printQuery(Query* pQuery, int size){
    for(int i = 0; i < size; i++){
        printf("%d\t", pQuery->type);
        printDate(pQuery->date1);
        printf("\t");
        printDate(pQuery->date2);
        printf("\t%s\n", pQuery->dataFileName);
    }
}

int companyNameComparator(const void* e1, const void* e2){
    return strcmp(((Event*)e1)->company_name, ((Event*)e2)->company_name);
}

void sortEventsByCompanyName(Event* pEvents, int size){
    qsort(pEvents, size, sizeof(Event), companyNameComparator);
}

int dateComparator(const void* e1, const void* e2){
    Date* pDate1 = &(((Event*)e1)->date);
    Date* pDate2 = &(((Event*)e2)->date);
    if(pDate1->year < pDate2->year){
        return -1;
    }else if(pDate1->year == pDate2->year){
        if(pDate1->month < pDate2->month){
            return -1;
        }else if(pDate1->month == pDate2->month){
            if(pDate1->day < pDate2->day){
                return -1;
            }else if(pDate1->day == pDate2->day){
                return 0;
            }else{
                return 1;
            }
        }else{
            return 1;
        }
    }else{
        return 1;
    }
}

void sortEventsByDate(Event* pEvents, int size){
    qsort(pEvents, size, sizeof(Event), dateComparator);
}

void calcDisplacementByCompanyName(Event* events, int eventCount, int* disp, int dispCount){
    int firstLetterCount[26];
    for(int i = 0; i < 26; i++){
        firstLetterCount[i] = 0;
    }

    for(int i = 0; i < eventCount; i++){
        firstLetterCount[events[i].company_name[0] - 65]++;
    }
//    for(int i = 0; i < 26; i++){
//        printf("%c\t%d\n", 'A' + i, firstLetterCount[i]);
//    }

    int tmpDisp[dispCount];
    for(int i = 0; i < dispCount; i++){
        tmpDisp[i] = 0;
    }

    int rem = 26 % dispCount;
    int defaultDispSize = 26 / dispCount;

    for(int i = 0; i < 26; i++){
        int rem_t = rem;
        int i_t = i;
        int dispIndex = -1;
        int dispSize;
        while(i_t >= 0){
            if(rem_t > 0){
                dispSize = defaultDispSize + 1;
                rem_t--;
            }else{
                dispSize = defaultDispSize;
            }
            i_t -= dispSize;

            dispIndex++;
        }
//        printf("%c\tdisp index:%d\n", 'A' + i, dispIndex);
        tmpDisp[dispIndex] += firstLetterCount[i];
    }

//    for(int i = 0; i < dispCount; i++){
//        printf("%d\ttmp disp:%d\n",i, tmpDisp[i]);
//    }

    disp[0] = 0;
//    printf("%d\tdisp:%d\n", 0, disp[0]);
    for(int i = 1; i < dispCount; i++){
        disp[i] = disp[i - 1] + tmpDisp[i - 1];
//        printf("%d\tdisp:%d\n",i, disp[i]);
    }
}


void mergeEvents(Event* allEvent, int* pAllEventCount, Event (*bucketEvents)[MAX_EVENT_SIZE], int* bucketEventCount, int bucketCount, int byDateOrCompanyName){
    int allEventCount = 0;
    int bucketEventCursor[bucketCount];
    for(int i = 0; i < bucketCount; i++){
        bucketEventCursor[i] = 0;
        allEventCount += bucketEventCount[i];
    }
    *pAllEventCount = allEventCount;

    Event* refEvent;
    int targetBucketIndex, targetEventIndex;

    int cmpResult;
    for(int i = 0; i < allEventCount; i++){
        for(int j = 0; j < bucketCount; j++){
            if(bucketEventCursor[j] < bucketEventCount[j]){
                refEvent = bucketEvents[j] + bucketEventCursor[j];
                targetBucketIndex = j;
                targetEventIndex = bucketEventCursor[j];
                break;
            }
        }

        for(int j = 0; j < bucketCount; j++){
            if(bucketEventCursor[j] < bucketEventCount[j]){
                if(byDateOrCompanyName){
                    cmpResult = strcmp(bucketEvents[j][bucketEventCursor[j]].company_name, refEvent->company_name);
                }else{
                    cmpResult = dateCmp(&(bucketEvents[j][bucketEventCursor[j]].date), &(refEvent->date));
                }

                if(cmpResult < 0){
                    targetBucketIndex = j;
                    targetEventIndex = bucketEventCursor[j];
                }
            }
        }
        allEvent[i] = bucketEvents[targetBucketIndex][targetEventIndex];
        bucketEventCursor[targetBucketIndex] += 1;
    }
}
