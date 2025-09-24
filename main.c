#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ค่าคงที่หลักโปรแกรม
#define MAX_RECORDS 200
#define STRLEN 64

// MAX_RECORDS: จำนวนเรคคอร์ดสูงสุดที่รองรับ
// STRLEN     : ความยาวสตริงสูงสุดของแต่ละฟิลด์
char policyNumber[MAX_RECORDS][STRLEN];
char ownerName   [MAX_RECORDS][STRLEN];
char carModel    [MAX_RECORDS][STRLEN];
char startDate   [MAX_RECORDS][STRLEN];
int  recCount = 0;

// ฟังก์ชันช่วยจัดการสตริงพื้นฐาน
// ลบ \n/\r ที่ท้ายสตริง (ใช้หลัง fgets)
void rtrim_newline(char *s) { 
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}
// ตัดช่องว่างหัว-ท้ายสตริง
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
// คัดลอก src -> dst พร้อมแปลงเป็นตัวพิมพ์เล็ก
void toLowerStr(const char *src, char *dst) {
    int i = 0;
    while (src[i] != '\0' && i < STRLEN - 1) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}
// คืนค่า 1 ถ้า s มี key (ไม่สนตัวพิมพ์), ไม่งั้น 0
int containsIgnoreCase(const char *s, const char *key) { 
    char s2[STRLEN], k2[STRLEN];
    toLowerStr(s, s2);
    toLowerStr(key, k2);
    if (k2[0] == '\0') return 1;
    return strstr(s2, k2) != NULL;
}

/* ค้นหาว่ามีเลขนี้แล้วหรือไม่
   คืนค่า index ถ้าพบ, -1 ถ้าไม่พบ (ใช้กันข้อมูลซ้ำ) */
int findPolicyExact(const char *num) {
    int i;
    for (i = 0; i < recCount; i++) {
        if (strcmp(policyNumber[i], num) == 0) {
            return i; // found
        }
    }
    return -1; // not found
}

/* ปรับเลขประกันให้เป็นรูปแบบมาตรฐาน
   รูปแบบที่รับ: ตัวอักษร 1 ตัว + ตัวเลข 1-3 หลัก (เช่น P23)
   ผลลัพธ์: แปลงเป็น ตัวอักษร + เลข 3 หลัก (เช่น P023)
   คืนค่า 1 ถ้าถูกต้อง, 0 ถ้าไม่ถูกต้อง */
int normalizePolicy(const char *in, char *out) {
    char c = '\0';
    int num = -1;
    int i = 0;
    char buf[STRLEN];

    strncpy(buf, in, STRLEN - 1);
    buf[STRLEN - 1] = '\0';
    trim_spaces(buf);
    if (buf[0] == '\0') return 0;

    // ตรวจว่าตัวแรกเป็นตัวอักษร A-Z
    c = (char)toupper((unsigned char)buf[0]); 
    if (!(c >= 'A' && c <= 'Z')) return 0;

    i = 1;
    if (buf[i] == '\0') return 0;
    // อ่านตัวเลข 1-3 หลัก แล้วเช็คช่วง 0-999
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
    // จัดรูปแบบเป็น %c%03d
    sprintf(out, "%c%03d", c, num);
    return 1;
}

/* แปลงวันที่ให้เป็นรูปแบบ YYYY-MM-DD
   อินพุต: "YYYY M D" หรือ "YYYY-M-D" (เช่น 2025 5 5 หรือ 2025-5-5)
   ตรวจช่วงปี/เดือน/วันแบบพื้นฐาน แล้วจัดรูปแบบเป็นสองหลัก */
int parseAndFormatDate(const char *in, char *out) {
    char buf[STRLEN];
    int y = 0, m = 0, d = 0;
    int i;

    strncpy(buf, in, STRLEN - 1); 
    buf[STRLEN - 1] = '\0';
    trim_spaces(buf);

    // แทนที่ '-' เป็นช่องว่าง เพื่อง่ายต่อ sscanf
    for (i = 0; buf[i] != '\0'; i++) {  
        if (buf[i] == '-') buf[i] = ' ';
    }
    // ตรวจช่วงค่า (ปี 1-9999, เดือน 1-12, วัน 1-31)
    if (sscanf(buf, "%d %d %d", &y, &m, &d) != 3) return 0; 
    if (y < 1 || y > 9999) return 0;
    if (m < 1 || m > 12) return 0;
    if (d < 1 || d > 31) return 0;

    sprintf(out, "%04d-%02d-%02d", y, m, d); 
    return 1;
}

/* จัดการแถวที่ดูเหมือน header
   - isHeaderLikeRow: เช็คชื่อคอลัมน์
   - purgeHeaderRows: ลบแถว header ออกจากหน่วยความจำและจัดอาเรย์ */
int isHeaderLikeRow(const char *p1, const char *p2, const char *p3, const char *p4) {
    return (strcmp(p1, "PolicyNumber") == 0) ||
           (strcmp(p2, "OwnerName")   == 0) ||
           (strcmp(p3, "CarModel")    == 0) ||
           (strcmp(p4, "StartDate")   == 0);
}

/* โหลดข้อมูลจากไฟล์ CSV
   ขั้นตอน:
   1) ข้าม header ถ้ามี
   2) อ่านทีละบรรทัดแยกฟิลด์ด้วย ','
   3) ข้ามแถวที่เป็น header/ไม่ครบ/ซ้ำ
   4) เพิ่มลงฐานข้อมูล และนับจำนวนแถวที่โหลดสำเร็จ */
void purgeHeaderRows(void) {
    int i, j = 0;
    for (i = 0; i < recCount; ++i) {
        if (isHeaderLikeRow(policyNumber[i], ownerName[i], carModel[i], startDate[i])) {
            continue;
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
    // แยกฟิลด์ 4 ช่อง: PolicyNumber, OwnerName, CarModel, StartDate
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

        // ข้ามถ้าซ้ำหรือเป็น header
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

    // หลังโหลด ลบheader-like row ที่อาจเล็ดรอดเข้ามา
    purgeHeaderRows();

    return loaded;
}

/* บันทึกข้อมูลลงไฟล์ CSV
   เขียนหัวตารางก่อน แล้ววนเขียนแต่ละเรคคอร์ด */
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

/* ส่วนแสดงผล 1 แถว และหัวตาราง เพื่อให้รูปแบบสม่ำเสมอ */
void printHeader() { // printHeader: พิมพ์หัวคอลัมน์และเส้นคั่น
    printf("\n%-14s | %-20s | %-20s | %-10s\n", "PolicyNumber", "OwnerName", "CarModel", "StartDate");
    printf("---------------+----------------------+----------------------+------------\n");
}
/* ส่วนแสดงผล 1 แถว และหัวตาราง เพื่อให้รูปแบบสม่ำเสมอ */
void printRecord(int i) { // printRecord: พิมพ์ 1 เรคคอร์ดตามคอลัมน์
    printf("%-14s | %-20s | %-20s | %-10s\n",
           policyNumber[i], ownerName[i], carModel[i], startDate[i]);
}

/* แสดงข้อมูลทั้งหมดในระบบ
   - ลบแถว header ออกจากหน่วยความจำ
   - พิมพ์หัวตาราง + ทุกรายการ
   - สรุปจำนวนทั้งหมดท้ายตาราง */ 
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

/* โหมดเพิ่มข้อมูล (Add)
   ลำดับ:
   1) รับเลขกรมธรรม์ -> ตรวจรูปแบบ/กันข้อมูลซ้ำ
   2) รับชื่อเจ้าของ (0=Back)
   3) รับรุ่นรถ (0=Back)
   4) รับวันที่เริ่ม -> แปลงเป็น YYYY-MM-DD
   5) เพิ่มลงฐานข้อมูล, บันทึก CSV อัตโนมัติ
   6) ถามต่อด้วย (y/n) จนกว่าจะตอบถูกฟอร์แมต */
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

            if (!normalizePolicy(num_in, num_norm)) { // ถ้า normalizePolicy ไม่ผ่าน ให้แจ้งและถามใหม่
                printf("Invalid format. Use one uppercase letter + 3 digits, e.g., P023.\n");
                continue;
            }
            if (findPolicyExact(num_norm) != -1) { // ถ้าพบซ้ำด้วย findPolicyExact ให้แจ้งและถามใหม่
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
            if (!parseAndFormatDate(date_in, date_fmt)) { // parseAndFormatDate ไม่ผ่าน ให้แจ้งและถามใหม่
                printf("Invalid date. YYYY-MM-DD Example: 2025 5 5 or 2025-5-5\n");
                continue;
            }
            break;
        }

        // Save to memory arrays 
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

        // บันทึกอัตโนมัติหลังเพิ่มสำเร็จ
        saveToCSVFile("policies.csv");

        // วนถาม "Add more? (y/n)" รับเฉพาะ y/n เท่านั้น
        {
            for (;;) {
                char ans[8];
                printf("\nAdd more? (y/n): "); 
                if (scanf(" %7s", ans) != 1) {
                    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                    puts("Invalid input. Please type 'y' or 'n'.\n");
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
                puts("Invalid input. Please type 'y' or 'n'.\n");
                // วนถามใหม่จนกว่าจะถูก
            }
        }
        }
    }
}

/* โหมดค้นหา (Search)
   - เลือกค้นหาได้ 3 แบบ: เลขกรมธรรม์ / ชื่อเจ้าของ / รุ่นรถ
   - แสดงหัวตาราง + ผลลัพธ์ที่ตรงเงื่อนไข
   - สรุปจำนวนที่พบด้วย "Found N record(s)." ทุกครั้ง */
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
                int i, count = 0; // นับจำนวนที่ตรงเงื่อนไขในตัวแปร count

                printf("\nPolicy number (0=Back)\n: ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(policyNumber[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("--------------------------------------------------------------------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n"); // ถ้า count == 0 ให้แจ้ง "(No results.)"
            }

        } else if (choice == 2) {
            while (1) {
                char key[STRLEN];
                int i, count = 0;

                printf("\nOwner name (0=Back)\n: ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(ownerName[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("--------------------------------------------------------------------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n");
            }

        } else if (choice == 3) {
            while (1) {
                char key[STRLEN];
                int i, count = 0;

                printf("\nCar model (0=Back)\n: ");
                if (scanf(" %63[^\n]", key) != 1) return;
                if (strcmp(key, "0") == 0) break;

                printHeader();
                for (i = 0; i < recCount; i++) {
                    if (containsIgnoreCase(carModel[i], key)) {
                        printRecord(i);
                        count++;
                    }
                }
                printf("--------------------------------------------------------------------------\n");
                printf("Found %d record(s).\n", count);
                if (count == 0) printf("(No results.)\n");
            }

        }
        else {
            printf("Invalid choice. Please try again.\n");
        }
    }
}

// หน้าเมนูหลักของโปรแกรม 
void printMenu() {
    printf("\n==== POLICY MANAGEMENT ====\n");
    printf("1) List all policies\n");
    printf("2) Add new policies\n");
    printf("3) Search policies\n");
    printf("0) Exit program\n");
}

/* main:
   - ถ้ามีไฟล์ policies.csv ให้โหลดอัตโนมัติ
   - วนลูปเมนูหลัก: List / Add / Search / Exit
   - ออกจากโปรแกรม: ยืนยัน (y/n) แบบเข้มงวด */
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
        // Exit confirm ask (y/n) 
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
