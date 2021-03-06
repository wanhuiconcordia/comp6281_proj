#ifndef EVENT_H
#define EVENT_H
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "date.h"
#include "tools.h"

#define UNKNOWN_QUERY 0
#define COMPANY_SALES 1
#define SALES_BY_DATE 2
#define DELETE_BY_DATE 3
#define USER_EXIT 4
#define PARENT_PROCESS_READY 5

typedef struct{
    int type;
    Date date1;
    Date date2;
    char dataFileName[10];
}Query;


//#pragma pack(push, 1)
typedef struct{
    int sales_id;
    Date date;
    int company_id;
    char company_name[40];
    float sales_total;
}Event;
//#pragma pack(pop)

void readEvent(FILE *pFile, int readEventCount, Event* events, int* pSize);
void parseEvent(Event* pEvent, char* line);
void printEvent(Event* pEvent, int size);
void initQueryType(MPI_Datatype *pQueryType);
void initEventType(MPI_Datatype *pEvent);

void printQuery(Query* pQuery, int size);
void mergeEvents(Event* allEvent, int* pAllEventCount, Event** bucketEvents, int* bucketEventCount, int bucketCount, int byDateOrCompanyName);

void calcDisplacementByDate(Event* events, int eventCount, int* sendCounts, int* disp, int dispCount, Date date1, Date date2);
void calcDisplacementByCompanyName(Event* pEvents, int eventCount, int* sendCounts, int* pDisplacements, int dispSize);

int companyNameComparator(const void* e1, const void* e2);
void sortEventsByCompanyName(Event* pEvents, int size);

int dateComparator(const void* e1, const void* e2);
void sortEventsByDate(Event* pEvents, int size);
#endif // EVENT_H
