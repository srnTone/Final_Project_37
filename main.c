#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// จองพื้นที่
#define MAX_RECORDS 1000
#define MAX_STR     64
#define MAX_LINE    1024

// ใช้ฐานข้อมูลแบบ parallel arrays
static char PolicyNumber[MAX_RECORDS][MAX_STR];
static char OwnerName  [MAX_RECORDS][MAX_STR];
static char CarModel   [MAX_RECORDS][MAX_STR];
static char StartDate  [MAX_RECORDS][MAX_STR];
static int  rec_count = 0;

// Utilities
// ลบ \n และ \r ท้าย string
static void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

// ตัดช่องว่างหัวท้าย
static void trim_spaces(char *s) {
    char *start = s;
    while (*start == ' ' || *start == '\t') start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t')) s[--n] = '\0';
}

// อ่านบรรทัดและตัดปลาย
static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) return 0;
    rstrip(buf);
    return 1;
}

// อ่าน int จากบรรทัด 
static int read_int_prompt(const char *prompt, int *out) {
    char line[64];
    if (prompt) fputs(prompt, stdout);
    if (!read_line(line, sizeof(line))) return 0;
    char *end; long v = strtol(line, &end, 10);
    if (end == line) return 0;
    *out = (int)v;
    return 1;
}

// ---------------- CSV ----------------

// แยก 1 บรรทัด CSV เป็น 4 ฟิลด์ 
static int parse_csv_line(char *line, char *p, char *o, char *c, char *d) {
    char *tok = strtok(line, ",");
    if (!tok) return 0; strncpy(p, tok, MAX_STR-1); p[MAX_STR-1]='\0'; trim_spaces(p);

    tok = strtok(NULL, ",");
    if (!tok) return 0; strncpy(o, tok, MAX_STR-1); o[MAX_STR-1]='\0'; trim_spaces(o);

    tok = strtok(NULL, ",");
    if (!tok) return 0; strncpy(c, tok, MAX_STR-1); c[MAX_STR-1]='\0'; trim_spaces(c);

    tok = strtok(NULL, ",\n\r");
    if (!tok) return 0; strncpy(d, tok, MAX_STR-1); d[MAX_STR-1]='\0'; trim_spaces(d);
    return 1;
}

// Load CSV PolicyNumber,OwnerName,CarModel,StartDate 
static int load_csv(const char *fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
        printf("[load_csv] cannot open file: %s\n", fname);
        return 0;
    }
    rec_count = 0;

    char line[MAX_LINE];
    int first = 1;
    while (fgets(line, sizeof(line), f)) {
        rstrip(line);
        if (line[0] == '\0') continue;

        char tmp[MAX_LINE];
        strncpy(tmp, line, sizeof(tmp)-1);
        tmp[sizeof(tmp)-1] = '\0';

        char p[MAX_STR], o[MAX_STR], c[MAX_STR], d[MAX_STR];
        if (!parse_csv_line(tmp, p, o, c, d)) {
            /* skip malformed lines */
            continue;
        }
//ข้ามถ้าบรรทัดแรกเหมือน header
        if (first) {
            first = 0;
            if (strcmp(p,"PolicyNumber")==0 &&
                strcmp(o,"OwnerName")==0 &&
                strcmp(c,"CarModel")==0 &&
                strcmp(d,"StartDate")==0) {
                continue;
            }
        }

        if (rec_count < MAX_RECORDS) {
            strncpy(PolicyNumber[rec_count], p, MAX_STR);
            strncpy(OwnerName  [rec_count], o, MAX_STR);
            strncpy(CarModel   [rec_count], c, MAX_STR);
            strncpy(StartDate  [rec_count], d, MAX_STR);
            PolicyNumber[rec_count][MAX_STR-1]='\0';
            OwnerName  [rec_count][MAX_STR-1]='\0';
            CarModel   [rec_count][MAX_STR-1]='\0';
            StartDate  [rec_count][MAX_STR-1]='\0';
            rec_count++;
        } else {
            break;
        }
    }
    fclose(f);
    return 1;
}

// Main Options
// ประกาศโปรโตไทป์
static int find_index_by_policy(const char *policy) {
    for (int i = 0; i < rec_count; ++i) {
        if (strcmp(PolicyNumber[i], policy) == 0) return i;
    }
    return -1;
}

// List All
static void list_all(void) {
    printf("\n%-12s | %-20s | %-16s | %-10s\n", "Policy", "Owner", "CarModel", "StartDate");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < rec_count; ++i) {
        printf("%-12s | %-20s | %-16s | %-10s\n",
               PolicyNumber[i], OwnerName[i], CarModel[i], StartDate[i]);
    }
}

// Search
static void search_policy(void) {
    for (;;) {
        printf("\n-- Search --\n");
        printf("1) By Policy Number\n");
        printf("2) By Owner Name (substring)\n");
        printf("0) Back to Menu\n");
        printf("Type: ");

        char line[32];
        int mode = -1;
        if (!fgets(line, sizeof(line), stdin)) return;
        sscanf(line, "%d", &mode);

        if (mode == 0) return;
        else if (mode == 1) {
            char key[MAX_STR];
            printf("Policy Number to search: ");
            if (!read_line(key, sizeof(key))) return;
            trim_spaces(key);

            int idx = find_index_by_policy(key);
            if (idx == -1) {
                puts("Not found.");
                printf("Continue searching? (Enter = continue, 0 = back): ");
                char ans[8]; if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return;
                continue;
            } else {
                printf("Found: %s | %s | %s | %s\n",
                       PolicyNumber[idx], OwnerName[idx], CarModel[idx], StartDate[idx]);
                printf("Continue searching? (Enter = continue, 0 = back): ");
                char ans[8]; if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return;
            }
        } else if (mode == 2) {
            char key[MAX_STR];
            printf("Owner name keyword: ");
            if (!read_line(key, sizeof(key))) return;
            trim_spaces(key);
            if (key[0] == '\0') { puts("Empty keyword."); continue; }

            int found = 0;
            for (int i = 0; i < rec_count; ++i) {
                if (strstr(OwnerName[i], key)) {
                    printf("%s | %s | %s | %s\n",
                           PolicyNumber[i], OwnerName[i], CarModel[i], StartDate[i]);
                    found = 1;
                }
            }
            if (!found) puts("Not found.");

            printf("Continue searching? (Enter = continue, 0 = back): ");
            char ans[8]; if (!read_line(ans, sizeof(ans))) return;
            if (ans[0] == '0') return;
        } else {
            puts("Wrong mode.");
        }
    }
}

// Display
static void display_menu(void) {
    puts("\n=== Policy Manager ===");
    puts("1) List all data");
    puts("2) Search");
    puts("0) Exit");
    printf("Type: ");
}

// UI
int main(void) {
    load_csv("policies.csv");

    for (;;) {
        display_menu();
        int choice;
        if (!read_int_prompt(NULL, &choice)) continue;

        switch (choice) {
            case 1: list_all(); break;
            case 2: search_policy(); break;
            case 0: {
                char line[16];
                printf("Exit program? (y/n): ");
                if (!read_line(line, sizeof(line))) break;
                if (line[0]=='y' || line[0]=='Y') return 0;
                break;
            }
            default: puts("Wrong menu choice.");
        }
    }
    return 0;
}
