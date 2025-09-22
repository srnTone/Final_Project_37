#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// ---------------- Config ----------------
#define MAX_RECORDS 1000
#define MAX_STR     64
#define MAX_LINE    1024

// ---------------- Data Model (STRUCT) ----------------
typedef struct {
    char policyNumber[MAX_STR];
    char ownerName   [MAX_STR];
    char carModel    [MAX_STR];
    char startDate   [MAX_STR]; 
} Policy;

Policy db[MAX_RECORDS];
int rec_count = 0;

// ---------------- Utilities ----------------
void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[--n] = '\0';
    }
}

void trim_spaces(char *s) {
    // ตัดหัว
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // ตัดท้าย
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t')) s[--n] = '\0';
}

int read_line(char *buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) return 0;
    rstrip(buf);
    return 1;
}

int read_int_prompt(const char *prompt, int *out) {
    char line[64];
    if (prompt) fputs(prompt, stdout);
    if (!read_line(line, sizeof(line))) return 0;
    char *end;
    long v = strtol(line, &end, 10);
    if (end == line) return 0;
    *out = (int)v;
    return 1;
}

// YYYY-MM-DD check เบาๆ
int is_date_yyyy_mm_dd(const char *s) {
    if (strlen(s) != 10) return 0;
    if (!(isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) &&
          isdigit((unsigned char)s[2]) && isdigit((unsigned char)s[3]) &&
          s[4]=='-' &&
          isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]) &&
          s[7]=='-' &&
          isdigit((unsigned char)s[8]) && isdigit((unsigned char)s[9]))) return 0;
    return 1;
}

// ---------------- CSV I/O ----------------
int parse_csv_line(char *line, char *num, char *name, char *model, char *date) {
    // line จะถูกแก้ให้เป็นหลาย ๆ สตริงด้วย strtok
    char *tok;
    // ช่อง 1
    tok = strtok(line, ",");
    if (!tok) return 0;
    trim_spaces(tok);
    strncpy(num, tok, MAX_STR-1); num[MAX_STR-1] = '\0';
    // ช่อง 2
    tok = strtok(NULL, ",");
    if (!tok) return 0;
    trim_spaces(tok);
    strncpy(name, tok, MAX_STR-1); name[MAX_STR-1] = '\0';
    // ช่อง 3
    tok = strtok(NULL, ",");
    if (!tok) return 0;
    trim_spaces(tok);
    strncpy(model, tok, MAX_STR-1); model[MAX_STR-1] = '\0';
    // ช่อง 4
    tok = strtok(NULL, ",");
    if (!tok) return 0;
    trim_spaces(tok);
    strncpy(date, tok, MAX_STR-1); date[MAX_STR-1] = '\0';
    return 1;
}

int load_csv(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    int count_before = rec_count;
    while (fgets(line, sizeof(line), fp)) {
        rstrip(line);
        if (line[0] == '\0') continue;
        char num[MAX_STR], name[MAX_STR], model[MAX_STR], date[MAX_STR];
        char tmp[MAX_LINE];
        strncpy(tmp, line, sizeof(tmp)-1); tmp[sizeof(tmp)-1] = '\0';
        if (!parse_csv_line(tmp, num, name, model, date)) continue;
        if (rec_count < MAX_RECORDS) {
            strncpy(db[rec_count].policyNumber, num, MAX_STR-1);
            db[rec_count].policyNumber[MAX_STR-1] = '\0';
            strncpy(db[rec_count].ownerName, name, MAX_STR-1);
            db[rec_count].ownerName[MAX_STR-1] = '\0';
            strncpy(db[rec_count].carModel, model, MAX_STR-1);
            db[rec_count].carModel[MAX_STR-1] = '\0';
            strncpy(db[rec_count].startDate, date, MAX_STR-1);
            db[rec_count].startDate[MAX_STR-1] = '\0';
            rec_count++;
        }
    }
    fclose(fp);
    return rec_count - count_before;
}

int save_csv(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    for (int i = 0; i < rec_count; ++i) {
        fprintf(fp, "%s,%s,%s,%s\n",
                db[i].policyNumber,
                db[i].ownerName,
                db[i].carModel,
                db[i].startDate);
    }
    fclose(fp);
    return 1;
}

// ---------------- Core ops ----------------
void print_header(void) {
    printf("%-14s | %-20s | %-20s | %-10s\n",
           "\nPolicyNumber", "OwnerName", "CarModel", "StartDate");
    printf("----------------------------------------------------------------\n");
}

void print_record(int i) {
    printf("%-14s | %-20s | %-20s | %-10s\n",
           db[i].policyNumber, db[i].ownerName, db[i].carModel, db[i].startDate);
}

void list_all(void) {
    if (rec_count == 0) { puts("(No Data)"); return; }
    print_header();
    for (int i = 0; i < rec_count; ++i) {
        print_record(i);
    }
}

int add_record(const char *num, const char *name, const char *model, const char *date) {
    if (rec_count >= MAX_RECORDS) return 0;
    strncpy(db[rec_count].policyNumber, num, MAX_STR-1);
    db[rec_count].policyNumber[MAX_STR-1] = '\0';
    strncpy(db[rec_count].ownerName, name, MAX_STR-1);
    db[rec_count].ownerName[MAX_STR-1] = '\0';
    strncpy(db[rec_count].carModel, model, MAX_STR-1);
    db[rec_count].carModel[MAX_STR-1] = '\0';
    strncpy(db[rec_count].startDate, date, MAX_STR-1);
    db[rec_count].startDate[MAX_STR-1] = '\0';
    rec_count++;
    return 1;
}

void add_policy(void) {
    char num[MAX_STR], name[MAX_STR], model[MAX_STR], date[MAX_STR];

    printf("Add Policy Data\nPolicy Number: "); if (!read_line(num, sizeof(num))) return; trim_spaces(num);
    if (num[0] == '\0') { puts("Can't Be Empty"); return; }

    printf("Owner Name: "); if (!read_line(name, sizeof(name))) return; trim_spaces(name);
    printf("Car Model: "); if (!read_line(model, sizeof(model))) return; trim_spaces(model);

    printf("Start Date (YYYY-MM-DD): ");
    if (!read_line(date, sizeof(date))) return; trim_spaces(date);
    if (!is_date_yyyy_mm_dd(date)) {
        puts("Wrong Format (Should be YYYY-MM-DD)");
        return;
    }

    if (add_record(num, name, model, date)) puts("Add Successfully");
    else puts("Data Base is Full");
}

int find_by_policy_sub(const char *key) {
    int found = 0;
    print_header();
    for (int i = 0; i < rec_count; ++i) {
        if (strstr(db[i].policyNumber, key)) {
            print_record(i);
            found++;
        }
    }
    return found;
}

int find_by_owner_sub(const char *key) {
    int found = 0;
    print_header();
    for (int i = 0; i < rec_count; ++i) {
        if (strstr(db[i].ownerName, key)) {
            print_record(i);
            found++;
        }
    }
    return found;
}

// search loop: อยู่ในเมนู search ต่อจนกด 0
void search_policy(void) {
    for (;;) {
        printf("\n---- Search ----\n");
        printf("1) By Policy Number\n");
        printf("2) By Owner Name\n");
        printf("0) Back to Menu\n");
        printf("Type: ");

        char line[32]; int mode = -1;
        if (!read_line(line, sizeof(line))) return;
        char *end; long v = strtol(line, &end, 10); if (end != line) mode = (int)v;

        if (mode == 0) return;
        else if (mode == 1) {
            char key[MAX_STR];
            printf("Policy number keyword: ");
            if (!read_line(key, sizeof(key))) return;
            trim_spaces(key);
            if (key[0] == '\0') { puts("Empty Keyword"); continue; }
            int n = find_by_policy_sub(key);
            if (n == 0) puts("(Not Found)");
        }
        else if (mode == 2) {
            char key[MAX_STR];
            printf("Owner name keyword: ");
            if (!read_line(key, sizeof(key))) return;
            trim_spaces(key);
            if (key[0] == '\0') { puts("Empty Keyword"); continue; }
            int n = find_by_owner_sub(key);
            if (n == 0) puts("(Not Found)");
        }
        else {
            puts("Wrong Menu");
        }
    }
}

// ---------------- UI ----------------
void display_menu(void) {
    puts("\n==== MENU ====");
    puts("1) All Policy List");
    puts("2) Add Data");
    puts("3) Search");
    //puts("4) Update");
    //puts("5) Delete");
    //puts("6) Save to CSV");
    puts("0) Exit Program");
}

int main(void) {
    int loaded = load_csv("policies.csv");
    if (loaded > 0) printf("Load CSV: %d List\n", loaded);

    for (;;) {
        display_menu();
        int choice = -1;
        if (!read_int_prompt("Choose Menu: ", &choice)) { puts("Type Incorrect Number."); continue; }
        switch (choice) {
            case 1: list_all(); break;
            case 2: add_policy(); break;
            case 3: search_policy(); break;
            //case 4: update_policy(); break;
            //case 5: delete_policy(); break;
            /*case 6:
                if (save_csv("policies.csv")) puts("Saved.");
                else puts("Save failed.");
                break;*/
            case 0: {
                char line[8];
                printf("Exit Program? (y/n): ");
                if (!read_line(line, sizeof(line))) break;
                if (line[0]=='y' || line[0]=='Y') return 0;
                break;
            }
            default: puts("Incorrect Menu");
        }
    }
    return 0;
}
