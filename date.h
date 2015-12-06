#ifndef DATE_H
#define DATE_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct{
    unsigned short year;
    unsigned short month;
    unsigned short day;
}Date;

void printDate(Date d);
int isLeapyear(unsigned short year);
Date generateDate(Date begin, Date end);
int dateCmp(Date* date1, Date* date2);
int isValidDate(Date date);
void swapDate(Date* pDate1, Date* pDate2);

int dayFromEpoch(Date date);
Date dateFromNumSinceEpoch(int n);
#endif // DATE_H
