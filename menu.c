#include "menu.h"

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
        clear();
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
