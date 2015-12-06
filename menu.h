#ifndef MENU_H
#define MENU_H
#include "date.h"
#include "event.h"
#define clear() printf("\033[H\033[J")
int inputNumber();
void dateMenu(Date* date1, Date* date2);
void mainMenu(Query* pQuery);

#endif // MENU_H
