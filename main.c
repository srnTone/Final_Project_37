#include <stdio.h>
#include <string.h>
#include <stdlib.h> //ใช้ strtol
#include <ctype.h>  //ใช้เพื่อตรวจสอบว่าเป็นตัวเลข isdigits

#define MAX_RECORDS 1000 // เก็บได้สูงสุด 1000 เรคคอร์ด
#define MAX_STR     64   // ความยาวสตริงต่อฟิลด์ (รวม '\0')
#define MAX_LINE    1024 // บัฟเฟอร์อ่าน 1 บรรทัดจากไฟล์ CSV

// ใช้ฐานข้อมูลแบบ parallel arrays
char PolicyNumber[MAX_RECORDS][MAX_STR];
char OwnerName   [MAX_RECORDS][MAX_STR];
char CarModel    [MAX_RECORDS][MAX_STR];
char StartDate   [MAX_RECORDS][MAX_STR];
int  rec_count = 0;

// --------------------- Utilities ---------------------
// ลบ \n และ \r ท้าย string
void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

// ตัดช่องว่างหัวท้าย 
void trim_spaces(char *s) {
    char *start = s;
    while (*start == ' ' || *start == '\t') start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t')) s[--n] = '\0';
}

// อ่านบรรทัดและตัดปลาย
int read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) return 0;
    rstrip(buf);
    return 1;
}

// อ่าน int จากบรรทัด
int read_int_prompt(const char *prompt, int *out) {
    char line[64];
    if (prompt) fputs(prompt, stdout);
    if (!read_line(line, sizeof(line))) return 0;
    char *end; long v = strtol(line, &end, 10);
    if (end == line) return 0;
    *out = (int)v;
    return 1;
}

// ----------- ตัวช่วยตรวจ format input ใน Add -----------
// PolicyNumber ต้องเป็น: อักษร A-Z 1 ตัว + ตัวเลข 3 ตัว รวม 4 ตัวอักษร
int is_valid_policy_number(const char *s) {
    if (!s) return 0;
    if (strlen(s) != 4) return 0;
    if (!(s[0] >= 'A' && s[0] <= 'Z')) return 0;
    for (int i = 1; i < 4; ++i) if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

/* แปลงอินพุตวันที่ที่พิมพ์แบบ "YYYY MM DD" หรือจะคั่นด้วย - ก็ได้ ให้เป็น "YYYY-MM-DD"
    YYYY = ตัวเลข 4 หลักเท่านั้น, MM = 0-12, DD = 1-31 */
int format_date_to_yyyy_mm_dd(const char *in, char *out, size_t outsz) {
    if (!in || !out || outsz == 0) return 0;

    // ดึงเลข 3 ชุดแรก ยอมรับตัวคั่นอะไรก็ได้ที่ไม่ใช่ตัวเลข
    char tmp[MAX_STR];
    size_t j = 0;
    int y = -1, m = -1, d = -1;
    int first_group_digits = 0;
    int in_digits = 0, first_done = 0;
    for (size_t i = 0; in[i] && j < sizeof(tmp) - 2; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (isdigit(c)) {
            tmp[j++] = c;
            if (!first_done) first_group_digits++;
            in_digits = 1;
        } else {
            if (in_digits) { tmp[j++] = ' '; in_digits = 0; first_done = 1; }
        }
    }
    if (in_digits) { tmp[j++] = ' '; }
    tmp[j] = '\0';

    if (sscanf(tmp, "%d %d %d", &y, &m, &d) != 3) return 0;
    if (first_group_digits != 4) return 0;            // ต้อง 4 หลักเป๊ะ
    if (m < 0 || m > 12) return 0;                    // ตามที่กำหนด: 0-12 
    if (d < 1 || d > 31) return 0;                    // 1-31 (ไม่เช็ควันจริงตามเดือน) 

    /* จัดรูปเก็บเป็น YYYY-MM-DD */
    snprintf(out, outsz, "%04d-%02d-%02d", y, m, d);
    return 1;
}

// ------------------------ CSV ------------------------
// แยก 1 บรรทัด CSV เป็น 4 ฟิลด์ 
int parse_csv_line(char *line, char *p, char *o, char *c, char *d) {
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
int load_csv(const char *fname) {
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

// ------------------- Main Options --------------------
// ประกาศโปรโตไทป์
int find_index_by_policy(const char *policy) {
    for (int i = 0; i < rec_count; ++i) {
        if (strcmp(PolicyNumber[i], policy) == 0) return i;
    }
    return -1;
}

// List All
void list_all(void) {
    printf("\n%-12s | %-20s | %-20s | %-10s\n", "Policy", "Owner", "CarModel", "StartDate");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < rec_count; ++i) {
        printf("%-12s | %-20s | %-20s | %-10s\n",
               PolicyNumber[i], OwnerName[i], CarModel[i], StartDate[i]);
    }
}

// Add 
// input ตรวจฟอร์แมต + ลูปถามทำต่อ/เลือกกลับเมนู
void add_policy(void) {
    for (;;) { // วนรอบ session การเพิ่มข้อมูล 
    restart_form:
        if (rec_count >= MAX_RECORDS) { puts("Database is full."); return; }

        //1) PolicyNumber
        for (;;) {
            char buf[MAX_STR];
            printf("\n----- Add new -----\nPolicy Number (A###) [0=cancel]\n: ");
            if (!read_line(buf, sizeof(buf))) { puts("Cancelled."); return; }
            trim_spaces(buf);

            if (strcmp(buf, "0") == 0) {
                // เลือกว่าจะเริ่มกรอกใหม่ทั้งฟอร์มหรือกลับเมนู 
                char ans[8];
                printf("Cancel this Add?\n(Enter = restart Add, 0 = back to menu): ");
                if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return;  // กลับเมนูหลัก 
                else goto restart_form;     // เริ่มฟอร์มใหม่ 
            }

            if (!is_valid_policy_number(buf)) {
                puts("Invalid Policy Number. Must be 1 uppercase letter + 3 digits (e.g., A123).");
                continue; // แจ้งแล้วให้กรอกใหม่ทันที 
            }
            if (find_index_by_policy(buf) != -1) {
                puts("This insurance number ALREADY EXISTS.");
                continue; // ซ้ำ -> แจ้งแล้วให้กรอกใหม่ทันที 
            }

            strncpy(PolicyNumber[rec_count], buf, MAX_STR-1);
            PolicyNumber[rec_count][MAX_STR-1] = '\0';
            break; // ออกไปกรอกฟิลด์ถัดไป 
        }

        //2) OwnerName
        for (;;) {
            printf("\nOwnerName [0=cancel]\n: ");
            if (!read_line(OwnerName[rec_count], MAX_STR)) { puts("Cancelled."); return; }
            trim_spaces(OwnerName[rec_count]);

            if (strcmp(OwnerName[rec_count], "0") == 0) {
                char ans[8];
                printf("Cancel this Add? (Enter = restart Add, 0 = back to menu): ");
                if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return; else goto restart_form;
            }
            break;
        }

        // 3) CarModel
        for (;;) {
            printf("\nCarModel [0=cancel]\n: ");
            if (!read_line(CarModel[rec_count], MAX_STR)) { puts("Cancelled."); return; }
            trim_spaces(CarModel[rec_count]);

            if (strcmp(CarModel[rec_count], "0") == 0) {
                char ans[8];
                printf("Cancel this Add? (Enter = restart Add, 0 = back to menu): ");
                if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return; else goto restart_form;
            }
            break;
        }

        // 4) StartDate
        for (;;) {
            char date_in[MAX_STR], date_fmt[MAX_STR];
            printf("\nStart Date [type: YYYY MM DD] [0=cancel]\n: ");
            if (!read_line(date_in, sizeof(date_in))) { puts("Cancelled."); return; }
            trim_spaces(date_in);

            if (strcmp(date_in, "0") == 0) {
                char ans[8];
                printf("Cancel this Add? (Enter = restart Add, 0 = back to menu): ");
                if (!read_line(ans, sizeof(ans))) return;
                if (ans[0] == '0') return; else goto restart_form;
            }
            if (!format_date_to_yyyy_mm_dd(date_in, date_fmt, sizeof(date_fmt))) {
                puts("Invalid date. Use: YYYY MM DD  (Year = 4 digits, Month = 0-12, Day = 1-31).");
                continue; // แจ้งแล้วให้กรอกวันใหม่ทันที 
            }

            strncpy(StartDate[rec_count], date_fmt, MAX_STR-1);
            StartDate[rec_count][MAX_STR-1] = '\0';
            break;
        }

        // บันทึก + แสดงรายการที่เพิ่งเพิ่ม
        int added_idx = rec_count;   // จด index ปัจจุบันไว้ก่อนเพิ่มตัวนับ
        rec_count++;

        puts("\nAdded.");
        puts("This is the record just added successfully:");
        printf("\n%-12s | %-20s | %-20s | %-10s\n", "Policy", "Owner", "CarModel", "StartDate");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-12s | %-20s | %-16s | %s\n",
               PolicyNumber[added_idx],
               OwnerName[added_idx],
               CarModel[added_idx],
               StartDate[added_idx]);

        printf("\nAdd another? (Enter = continue, 0 = back): ");
        {
            char ans[8];
            if (!read_line(ans, sizeof(ans))) return;
            if (ans[0] == '0') return;
        }
    }
}

// Search 
// ค้นหาจากบางส่วนจากเลขกรมธรรม์หรือชื่อ เลือกหาต่อหรือเปลี่ยนโหมด
void search_policy(void) {
    for (;;) {
        printf("\n------ Search ------\n");
        printf("1) By Policy Number\n");
        printf("2) By Owner Name\n");
        printf("0) Back to Menu\n");
        printf("Type: ");

        char line[32];
        int mode = -1;
        if (!fgets(line, sizeof(line), stdin)) return;
        sscanf(line, "%d", &mode);  // ไม่ต้องมี \n

        if (mode == 0) return;

        else if (mode == 1) {
            //ค้นหาเลขกรมธรรม์บางส่วนและอยู่ต่อ
            for (;;) {
                char key[MAX_STR];
                printf("\nPolicy number keyword [0 = back]: ");
                if (!read_line(key, sizeof(key))) return;
                trim_spaces(key);
                if (strcmp(key, "0") == 0) break;              // กลับไปหน้าเลือกโหมด
                if (key[0] == '\0') { puts("Empty keyword."); continue; }

                int found = 0;
                printf("\n%-12s | %-20s | %-20s | %-10s\n", "Policy", "Owner", "CarModel", "StartDate");
                printf("-------------------------------------------------------------------------------\n");
                for (int i = 0; i < rec_count; ++i) {
                    if (strstr(PolicyNumber[i], key)) {       // ค้นบางส่วน
                        printf("%-12s | %-20s | %-20s | %-10s\n",
                               PolicyNumber[i], OwnerName[i], CarModel[i], StartDate[i]);
                        found = 1;
                    }
                }
                if (!found) puts("Not found.");
                // ไม่ถามยืนยัน—วนให้พิมพ์คำค้นใหม่ได้ทันที
            }
        }

        else if (mode == 2) {
            // ค้นหาชื่อแบบบางส่วนและอยู่ต่อ
            for (;;) {
                char key[MAX_STR];
                printf("\nOwner name keyword [0 = back]: ");
                if (!read_line(key, sizeof(key))) return;
                trim_spaces(key);
                if (strcmp(key, "0") == 0) break;              // กลับไปหน้าเลือกโหมด
                if (key[0] == '\0') { puts("Empty keyword."); continue; }

                int found = 0;
                printf("\n%-12s | %-20s | %-20s | %-10s\n", "Policy", "Owner", "CarModel", "StartDate");
                printf("-------------------------------------------------------------------------------\n");
                for (int i = 0; i < rec_count; ++i){
                    if (strstr(OwnerName[i], key)) {
                        printf("%-12s | %-20s | %-20s | %-10s\n",
                               PolicyNumber[i], OwnerName[i], CarModel[i], StartDate[i]);
                        found = 1;
                    }
                }
                if (!found) puts("Not found.");
            }
        }

        else {
            puts("Wrong mode.");
        }
    }
}

// ------------------- Display Menu -------------------
void display_menu(void) {
    puts("\n=== Policy Manager ===");
    puts("1) List all data");
    puts("2) Add");
    puts("3) Search");
//    puts("4) Update Start Date");
//    puts("5) Delete");
//    puts("6) Save to CSV");
    puts("0) Exit");
    printf("Type: ");
}

// ------------------------ UI ------------------------
int main(void) {
    load_csv("policies.csv");

    for (;;) {
        display_menu();
        int choice;
        if (!read_int_prompt(NULL, &choice)) continue;

        switch (choice) {
            case 1: list_all(); break;
            case 2: add_policy(); break;
            case 3: search_policy(); break;
//            case 4: update_policy(); break;
//            case 5: delete_policy(); break;            
/*            case 6:
                if (save_csv("policies.csv")) puts("Saved.");
                else puts("Save failed.");
                break;*/
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

