#include "date.h"

int dateCmp(Date* date1, Date* date2){
    return date1->year * 10000 + date1->month * 100 + date1->day - (date2->year * 10000 + date2->month * 100 + date2->day);
}

void swapDate(Date* pDate1, Date* pDate2){
    unsigned short t;
    t = pDate1->year;
    pDate1->year = pDate2->year;
    pDate2->year = t;

    t = pDate1->month;
    pDate1->month = pDate2->month;
    pDate2->month = t;

    t = pDate1->day;
    pDate1->day = pDate2->day;
    pDate2->day = t;
}

Date generateDate(Date begin, Date end){
    if(!isValidDate(begin)){
        printf("Bad date.");
        printDate(begin);
        return begin;
    }

    if(!isValidDate(end)){
        printf("Bad date.");
        printDate(end);
        return end;
    }

    Date newDate;
    if(dateCmp(&begin, &end) > 0){
        swapDate(&begin, &end);
    }
    short yearDiff = end.year - begin.year;
    if(begin.year == end.year){
        newDate.year = begin.year;
    }else{
        newDate.year = rand() % (yearDiff + 1) + begin.year;
    }

    if(newDate.year == begin.year){
        newDate.month = rand() % (13 - begin.month) + begin.month;
    }else if(newDate.year == end.year){
        newDate.month = rand() % end.month + 1;
    }else{
        newDate.month = rand() % 12 + 1;
    }

    unsigned short maxDay;

    if(newDate.month == 1
            || newDate.month == 3
            || newDate.month == 5
            || newDate.month == 7
            || newDate.month == 8
            || newDate.month == 10
            || newDate.month == 12){
        maxDay = 31;
    }else{
        if(newDate.month == 2
                && isLeapyear(newDate.year)){
            maxDay = 28;
        }else{
            maxDay = 30;
        }
    }

    if(newDate.year == begin.year && newDate.month == begin.month){
        newDate.day = rand() % (maxDay - begin.day + 1) + begin.day;
    }else if(newDate.year == end.year && newDate.month == end.month){
        newDate.day = rand() % end.day + 1;
    }else{
        newDate.day = rand() % maxDay + 1;
    }

    return newDate;
}

void printDate(Date date){
    printf("%4d-%02d-%02d", date.year, date.month, date.day);
}
int isLeapyear(unsigned short year){
    return !(year % 4 || !(year % 400));
}

int isValidDate(Date date){
    if(date.month < 1
            || date.month > 12){
        return 0;
    }else if(date.month == 2
             && isLeapyear(date.year)){
        if(date.day > 28){
            return 0;
        }
    }else if(date.month == 1
             || date.month == 3
             || date.month == 5
             || date.month == 7
             || date.month == 8
             || date.month == 10
             || date.month == 12){
        if(date.day > 31){
            return 0;
        }
    }else{
        if(date.day > 30){
            return 0;
        }
    }

    if(date.day < 1){
        return 0;
    }
    return 1;
}

int dayFromEpoch(Date date){
    int days = 0;
    for(int i = 0; i < date.year; i++){
        if(isLeapyear(i)){
            days += 366;
        }else{
            days += 365;
        }
    }

    switch(date.month){
    case 12:
        days = days + date.day -1;
    case 11:
        if(date.month == 11){
            days = days + date.day -1;
        }else{
            days += 30;
        }
    case 10:
        if(date.month == 10){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    case 9:
        if(date.month == 9){
            days = days + date.day -1;
        }else{
            days += 30;
        }
    case 8:
        if(date.month == 8){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    case 7:
        if(date.month == 7){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    case 6:
        if(date.month == 6){
            days = days + date.day -1;
        }else{
            days += 30;
        }
    case 5:
        if(date.month == 5){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    case 4:
        if(date.month == 4){
            days = days + date.day -1;
        }else{
            days += 30;
        }
    case 3:
        if(date.month == 3){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    case 2:
        if(date.month == 2){
            days = days + date.day -1;
        }else{
            if(isLeapyear(date.year))
                days += 29;
            else
                days += 28;
        }
    case 1:
        if(date.month == 1){
            days = days + date.day -1;
        }else{
            days += 31;
        }
    }
    return days;
}

Date dateFromNumSinceEpoch(int n){
    Date date = {0, 1, 1};
    while(n > 365){
        if(isLeapyear(date.year)){
            n -= 366;
        }else{
            n -= 365;
        }
        date.year++;
    }

//    printf("year: %d, remain day:%d\n", date.year, n);
    if(n == 365){
        if(isLeapyear(date.year)){
            date.month = 12;
            date.day = 31;
            n = 0;
        }else{
            date.year++;
            n -= 365;
        }
    }

    if(n > 0){  //Jan
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(isLeapyear(date.year)){
        if(n > 0){  //Feb
            if(n > 29){
                date.month++;
                n -= 29;
            }else if(n == 29){
                date.month++;
                n = 0;
            }else{
                date.day += n;
                n = 0;
            }
        }
    }else{
        if(n > 0){  //Feb
            if(n > 28){
                date.month++;
                n -= 28;
            }else if(n == 28){
                date.month++;
                n = 0;
            }else{
                date.day += n;
                n = 0;
            }
        }
    }
    if(n > 0){  //Mar
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //Apr
        if(n > 30){
            date.month++;
            n -= 30;
        }else if(n == 30){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //May
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //June
        if(n > 30){
            date.month++;
            n -= 30;
        }else if(n == 30){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //July
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //Aug
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }
    if(n > 0){  //Sep
        if(n > 30){
            date.month++;
            n -= 30;
        }else if(n == 30){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }


    if(n > 0){  //Oct
        if(n > 31){
            date.month++;
            n -= 31;
        }else if(n == 31){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //Nov
        if(n > 30){
            date.month++;
            n -= 30;
        }else if(n == 30){
            date.month++;
            n = 0;
        }else{
            date.day += n;
            n = 0;
        }
    }

    if(n > 0){  //Dec
        date.day += n;
        n = 0;
    }
    return date;
}
