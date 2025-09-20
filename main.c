#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 1000
#define MAX_STR 64

static char PolicyNumber[MAX_RECORDS][MAX_STR];
static char OwnerName [MAX_RECORDS][MAX_STR];
static char CarModel  [MAX_RECORDS][MAX_STR];
static char StartDate [MAX_RECORDS][MAX_STR];
static int rec_count = 0;

static void display_menu(void) {
    puts("\n=== Insurance Manager ===");
    puts("1) List All DAta");
    puts("2) Add New");
    puts("3) Search");
    puts("4) Update Start Date");
    puts("5) Delete");
    puts("6) Save To CSV");
    puts("0) Exit");
    printf("Choose: ");
}

static void list_all(void) { puts("(No Data)"); }

int main(void) {
    int choice;
    for (;;) {
        display_menu();
        if (scanf("%d",&choice)!=1) break;
        if (choice==0) break;
        puts("WIP");
    }
    return 0;
}