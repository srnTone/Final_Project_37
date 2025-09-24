#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_RECORDS 200
#define STRLEN 64

char policyNumber[MAX_RECORDS][STRLEN];
char ownerName   [MAX_RECORDS][STRLEN];
char carModel    [MAX_RECORDS][STRLEN];
char startDate   [MAX_RECORDS][STRLEN];
int  recCount = 0;

void rtrim_newline(char *s) { 
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

void trim_spaces(char *s) {
    int i = 0, j, start = 0, end;
    while (s[i] == ' ' || s[i] == '\t') i++;
    start = i;
    end = (int)strlen(s) - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t')) end--;
    if (start > 0) {
        for (j = start; j <= end; j++) s[j - start] = s[j];
    }
    s[end - start + 1] = '\0';
}

void toLowerStr(const char *src, char *dst) {
    int i = 0;
    while (src[i] != '\0' && i < STRLEN - 1) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}
 
int containsIgnoreCase(const char *s, const char *key) { 
    char s2[STRLEN], k2[STRLEN];
    toLowerStr(s, s2);
    toLowerStr(key, k2);
    if (k2[0] == '\0') return 1;
    return strstr(s2, k2) != NULL;
}

// [POLICY] เช็คซ้ำ -> ยกเลิกถ้า findPolicyExact(num_norm) != -1
int findPolicyExact(const char *num) {
    int i;
    for (i = 0; i < recCount; i++) {
        if (strcmp(policyNumber[i], num) == 0) {
            return i; // found
        }
    }
    return -1; // not found
}

// [POLICY] normalize & validate -> ensure [A-Z][000-999], auto pad -> P023
int normalizePolicy(const char *in, char *out) {
    char c = '\0';
    int num = -1;
    int i = 0;
    char buf[STRLEN];

    // copy input to buffer
    strncpy(buf, in, STRLEN - 1);
    buf[STRLEN - 1] = '\0';
    trim_spaces(buf);
    if (buf[0] == '\0') return 0;

    // input ต้องเป็น A-Z
    c = (char)toupper((unsigned char)buf[0]); 
    if (!(c >= 'A' && c <= 'Z')) return 0;

    i = 1;
    if (buf[i] == '\0') return 0;
    // อนุญาต 1-3 digits, เต็มที่ 000-999 
    {
        int dcount = 0;
        int n = 0;
        while (buf[i] != '\0' && dcount < 3) {
            if (buf[i] < '0' || buf[i] > '9') return 0;
            n = n * 10 + (buf[i] - '0');
            dcount++;
            i++;
        }
        if (buf[i] != '\0') return 0;  // ตรวจสอบว่ามีตัวอื่นนอกจากตัวเลข
        if (dcount < 1) return 0;     
        if (n < 0 || n > 999) return 0;
        num = n;
    }

    sprintf(out, "%c%03d", c, num);
    return 1;
}

// [DATE] รับ format -> ยอมรับแค่ "YYYY M D" หรือ "YYYY-M-D", แปลงเป็น "YYYY-MM-DD"
int parseAndFormatDate(const char *in, char *out) {
    char buf[STRLEN];
    int y = 0, m = 0, d = 0;
    int i;

    // copy input to buffer
    strncpy(buf, in, STRLEN - 1); 
    buf[STRLEN - 1] = '\0';
    trim_spaces(buf);

    // แทนที่ - ด้วย space
    for (i = 0; buf[i] != '\0'; i++) {  
        if (buf[i] == '-') buf[i] = ' ';
    }
    // format input to YYYY-MM-DD
    if (sscanf(buf, "%d %d %d", &y, &m, &d) != 3) return 0; 
    if (y < 1 || y > 9999) return 0;
    if (m < 1 || m > 12) return 0;
    if (d < 1 || d > 31) return 0;

    sprintf(out, "%04d-%02d-%02d", y, m, d); 
    return 1;
}

// ==== NEW: helper to detect header-like rows ====
int isHeaderLikeRow(const char *p1, const char *p2, const char *p3, const char *p4) {
    return (strcmp(p1, "PolicyNumber") == 0) ||
           (strcmp(p2, "OwnerName")   == 0) ||
           (strcmp(p3, "CarModel")    == 0) ||
           (strcmp(p4, "StartDate")   == 0);
}

// ==== NEW: purge header-like rows from memory ====
void purgeHeaderRows(void) {
    int i, j = 0;
    for (i = 0; i < recCount; ++i) {
        if (isHeaderLikeRow(policyNumber[i], ownerName[i], carModel[i], startDate[i])) {
            continue; // skip header-looking row
        }
        if (j != i) {
            strncpy(policyNumber[j], policyNumber[i], STRLEN);
            strncpy(ownerName[j],    ownerName[i],    STRLEN);
            strncpy(carModel[j],     carModel[i],     STRLEN);
            strncpy(startDate[j],    startDate[i],    STRLEN);
            policyNumber[j][STRLEN - 1] = '\0';
            ownerName[j][STRLEN - 1]    = '\0';
            carModel[j][STRLEN - 1]     = '\0';
            startDate[j][STRLEN - 1]    = '\0';
        }
        ++j;
    }
    recCount = j;
}

int loadFromCSVFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    char line[512];
    int loaded = 0;
    if (fp == NULL) {
        printf("Cannot open %s for reading.\n", filename);
        return 0;
    }

    // ข้าม header ถ้ามี
    long pos = ftell(fp);
    if (fgets(line, sizeof(line), fp)) {
        rtrim_newline(line);
        if (strstr(line, "PolicyNumber") == NULL) {
            fseek(fp, pos, SEEK_SET);
        }
    } else {
        fclose(fp);
        printf("Empty file: %s\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *p1, *p2, *p3, *p4;
        rtrim_newline(line);
        if (line[0] == '\0') continue;

        p1 = strtok(line, ",");
        p2 = strtok(NULL, ",");
        p3 = strtok(NULL, ",");
        p4 = strtok(NULL, ",");

        if (!p1 || !p2 || !p3 || !p4) continue;
        if (recCount >= MAX_RECORDS) { printf("Database full. Some rows were not loaded.\n"); break; }

        // Skip any row that looks like a header (defensive)
        if (strcmp(p1, "PolicyNumber") == 0 || strcmp(p2, "OwnerName") == 0 ||
            strcmp(p3, "CarModel") == 0 || strcmp(p4, "StartDate") == 0) {
            continue;
        }

        // Avoid duplicate policy numbers 
        if (findPolicyExact(p1) != -1) continue;

        strncpy(policyNumber[recCount], p1, STRLEN - 1);
        policyNumber[recCount][STRLEN - 1] = '\0';
        strncpy(ownerName[recCount], p2, STRLEN - 1);
        ownerName[recCount][STRLEN - 1] = '\0';
        strncpy(carModel[recCount], p3, STRLEN - 1);
        carModel[recCount][STRLEN - 1] = '\0';
        strncpy(startDate[recCount], p4, STRLEN - 1);
        startDate[recCount][STRLEN - 1] = '\0';

        recCount++;
        loaded++;
    }

    fclose(fp);
    printf("Loaded %d record(s) from %s\n", loaded, filename);

    // หลังโหลด ลบ header-like row ที่อาจเล็ดรอดเข้ามา
    purgeHeaderRows();

    return loaded;
}

void saveToCSVFile(const char *filename) {
    FILE *fp = fopen(filename, "w");
    int i;
    if (fp == NULL) {
        printf("Save failed (cannot open file).\n");
        return;
    }
    fprintf(fp, "PolicyNumber,OwnerName,CarModel,StartDate\n");
    for (i = 0; i < recCount; i++) {
        fprintf(fp, "%s,%s,%s,%s\n",
                policyNumber[i], ownerName[i], carModel[i], startDate[i]);
    }
    fclose(fp);
    printf("Saved to %s successfully\n", filename);
}

// print header
void printHeader() {
    printf("\n%-14s | %-20s | %-20s | %-10s\n", "PolicyNumber", "OwnerName", "CarModel", "StartDate");
    printf("---------------+----------------------+----------------------+------------\n");
}
// print record จาก array
void printRecord(int i) { 
    printf("%-14s | %-20s | %-20s | %-10s\n",
           policyNumber[i], ownerName[i], carModel[i], startDate[i]);
}

// list all 
void listAll() { 
    int i;
    if (recCount == 0) {
        printf("\n(No policies yet.)\n");
        return;
    }

    // ลบแถวที่ชื่อเหมือน header ออกจากหน่วยความจำก่อนแสดงผล
    purgeHeaderRows();

    printHeader();
    for (i = 0; i < recCount; i++) {
        printRecord(i);
    }
    printf("--------------------------------------------------------------------------\n");
    printf("Total policies: %d\n", recCount);
}

// Add mode
void addMode() {
    while (1) {
        char num_in[STRLEN], num_norm[STRLEN];
        char name[STRLEN];
        char model[STRLEN];
        char date_in[STRLEN], date_fmt[STRLEN];

        if (recCount >= MAX_RECORDS) {
            printf("\nDatabase is full. Cannot add more.\n");
            return;
        }

        // Policy Number
        for (;;) {
            printf("\nEnter policy number (A123) (0=Back)\n: ");
            if (scanf(" %63[^\n]", num_in) != 1) return;
            if (strcmp(num_in, "0") == 0) return;

            if (!normalizePolicy(num_in, num_norm)) {
                printf("Invalid format. Use one uppercase letter + 3 digits, e.g., P023.\n");
                continue;
            }
            if (findPolicyExact(num_norm) != -1) {
                printf("Duplicate policy number (%s). Use a different one.\n", num_norm);
                continue;
            }
            break;
        }
        // Owner Name
        printf("Enter owner name (0=Back)\n: ");
        if (scanf(" %63[^\n]", name) != 1) return;
        if (strcmp(name, "0") == 0) return;

        // Car Model
        printf("Enter car model (0=Back)\n: ");
        if (scanf(" %63[^\n]", model) != 1) return;
        if (strcmp(model, "0") == 0) return;
        
        // Start Date
        for (;;) {
            printf("Enter start date (YYYY-MM-DD) (0=Back)\n: ");
            if (scanf(" %63[^\n]", date_in) != 1) return;
            if (strcmp(date_in, "0") == 0) return;
            if (!parseAndFormatDate(date_in, date_fmt)) {
                printf("Invalid date. YYYY-MM-DD Example: 2025 5 5 or 2025-5-5\n");
                continue;
            }
            break;
        }

        // [SAVE] to memory arrays -> policyNumber[..], ownerName[..], ...
        strncpy(policyNumber[recCount], num_norm, STRLEN - 1);
        policyNumber[recCount][STRLEN - 1] = '\0';
        strncpy(ownerName[recCount], name, STRLEN - 1);
        ownerName[recCount][STRLEN - 1] = '\0';
        strncpy(carModel[recCount], model, STRLEN - 1);
        carModel[recCount][STRLEN - 1] = '\0';
        strncpy(startDate[recCount], date_fmt, STRLEN - 1);
        startDate[recCount][STRLEN - 1] = '\0';

        recCount++;
        printf("Added! (%s)\n", num_norm);

        // [AUTO-SAVE CSV] saveToCSVFile("policies.csv") immediately after success
        saveToCSVFile("policies.csv");

        // Ask to add more
        {
            for (;;) {
                char ans[8];
                printf("\nAdd more? (y/n): ");
                if (scanf(" %7s", ans) != 1) {
                    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                    puts("Invalid input. Please type 'y' or 'n'.");
                    continue;
            }
            // รับเฉพาะ 'y' หรือ 'n' เป็นตัวเดียวเท่านั้น
            if ((ans[0] == 'y' || ans[0] == 'Y') && ans[1] == '\0') {
                // y = เพิ่มต่อ -> ออกจากลูปถามกลับไปวน add รอบถัดไป
                break;
            } else if ((ans[0] == 'n' || ans[0] == 'N') && ans[1] == '\0') {
                // n = ออกจากโหมด add
                return;
            } else {
                puts("Invalid input. Please type 'y' or 'n'.");
                // วนถามใหม่จนกว่าจะถูก
            }
        }
        }
    }
}

// Search mode
void searchMode() {
    while (1) {
        int choice;
        printf("\n------ Search ------\n");
        printf("1) By policy number\n");
        printf("2) By owner name\n");
        printf("3) By car model\n");
        printf("0) Back to main menu\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            printf("Invalid input.\n");
            continue;
        }

        if (choice == 0) return;
        else if (choice == 1) {
            while (1) {
                char key[STRLEN];
                int i, count = 0;

                printf("\nPolicy number (0=Back): ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(policyNumber[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("---------------+----------------------+----------------------+------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n");
            }

        } else if (choice == 2) {
            while (1) {
                char key[STRLEN];
                int i, count = 0;

                printf("\nOwner name (0=Back): ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(ownerName[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("---------------+----------------------+----------------------+------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n");
            }

        } else if (choice == 3) {
            while (1) {
                char key[STRLEN];
                int i, count = 0;

                printf("\nCar model (0=Back): ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(carModel[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("---------------+----------------------+----------------------+------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n");
            }

        }
        else {
            printf("Invalid choice. Please try again.\n");
        }
    }
}

// [MAIN LOOP] printMenu -> switch choice -> list/add/search
void printMenu() {
    printf("\n==== POLICY MANAGEMENT ====\n");
    printf("1) List all policies\n");
    printf("2) Add new policies\n");
    printf("3) Search policies\n");
    printf("0) Exit program\n");
}

int main() {
    // Auto-load on start (if file exists) 
    FILE *test = fopen("policies.csv", "r");
    if (test) {
        fclose(test);
        // [AUTO-LOAD CSV] if "policies.csv" exists -> loadFromCSVFile("policies.csv")
        loadFromCSVFile("policies.csv");
    } else {
        printf("Tip: place a file named policies.csv next to the program to auto-load.\n");
    }

    while (1) {
        int choice;
        printMenu();
        printf("Choose Menu: ");
        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            printf("Invalid input.\n");
            continue;
        }

        if (choice == 1) {
            listAll();
        } else if (choice == 2) {
            addMode();
        } else if (choice == 3) {
            searchMode();
        // [EXIT CONFIRM] ask (y/n)
        } else if (choice == 0) {
            for (;;) {
            char ans[8];
            printf("Are you sure you want to exit? (y/n): ");
            if (scanf(" %7s", ans) != 1) {
                int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                puts("Invalid input. Please type 'y' or 'n'.");
                continue;
            }
            // รับเฉพาะ 'y' หรือ 'n' เป็นตัวเดียวเท่านั้น
            if ((ans[0] == 'y' || ans[0] == 'Y') && ans[1] == '\0') {
                // y = ออกจากโปรแกรมทันที
                return 0;
            } else if ((ans[0] == 'n' || ans[0] == 'N') && ans[1] == '\0') {
                // n = กลับไปหน้าเมนูหลัก
                break;
            } else {
                puts("Invalid input. Please type 'y' or 'n'.\n");
                // วนถามใหม่จนกว่าจะถูก
            }
            }
        } else {
            printf("Incorrect menu.\n");
        }
    }
    return 0;
}
