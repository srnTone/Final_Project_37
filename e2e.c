// e2e.c — Standalone E2E without touching other files
// เทคนิค: rename main() ของ main.c เป็น app_main ชั่วคราว แล้ว include main.c
// ทำให้เข้าถึงฟังก์ชัน/ตัวแปรจริงทั้งหมด โดยไม่ต้องแก้ main.c

#include <stdio.h>
#include <string.h>

// 1) ดึงโค้ดจริงจากโปรเจกต์เข้ามา (พร้อมรีเนม main)
#define main app_main
#include "main.c"      // <-- ต้องวาง e2e.c ไว้โฟลเดอร์เดียวกับ main.c
#undef main

// ถึงตอนนี้ เรามี: MAX_RECORDS, STRLEN, policyNumber, ownerName, carModel, startDate, recCount
// และฟังก์ชันทั้งหมด (normalizePolicy, parseAndFormatDate, loadFromCSVFile, saveToCSVFile, ...)

// 2) ตัวช่วย assert (self-validating)
int T_TOTAL = 0, T_FAIL = 0;

void assert_equal_int(int exp, int act, const char *name) {
    T_TOTAL++;
    if (exp != act) { printf("✗ %s FAILED: expected=%d actual=%d\n", name, exp, act); T_FAIL++; }
    else            { printf("✓ %s PASSED\n", name); }
}
void assert_equal_str(const char *exp, const char *act, const char *name) {
    T_TOTAL++;
    if (strcmp(exp, act) != 0) { printf("✗ %s FAILED: expected=\"%s\" actual=\"%s\"\n", name, exp, act); T_FAIL++; }
    else                       { printf("✓ %s PASSED\n", name); }
}

// 3) helpers สำหรับทดสอบแบบไม่โต้ตอบ (ใช้โครงสร้างข้อมูลจริงจาก main.c)
void resetDB(void) {
    for (int i = 0; i < recCount; ++i) {
        policyNumber[i][0] = ownerName[i][0] = carModel[i][0] = startDate[i][0] = '\0';
    }
    recCount = 0;
}
int file_exists(const char *fn) {
    FILE *fp = fopen(fn, "r");
    if (fp) { fclose(fp); return 1; }
    return 0;
}
int count_csv_data_rows(const char *fn) {
    FILE *fp = fopen(fn, "r");
    if (!fp) return -1;
    char line[512];
    int count = 0, seen_header = 0;
    while (fgets(line, sizeof(line), fp)) {
        size_t n = strlen(line);
        while (n && (line[n-1]=='\n' || line[n-1]=='\r')) line[--n] = '\0';
        if (!n) continue;
        if (!seen_header && strstr(line, "PolicyNumber") == line) { seen_header = 1; continue; }
        count++;
    }
    fclose(fp);
    return count;
}
int addPolicyDirect(const char *num_in,
                           const char *owner,
                           const char *model,
                           const char *date_in)
{
    char num_norm[STRLEN], date_fmt[STRLEN];
    if (!normalizePolicy(num_in, num_norm)) return 0;
    if (findPolicyExact(num_norm) != -1)    return 0;
    if (!parseAndFormatDate(date_in, date_fmt)) return 0;

    strncpy(policyNumber[recCount], num_norm, STRLEN-1); policyNumber[recCount][STRLEN-1]='\0';
    strncpy(ownerName[recCount],    owner,    STRLEN-1); ownerName[recCount][STRLEN-1]   ='\0';
    strncpy(carModel[recCount],     model,    STRLEN-1); carModel[recCount][STRLEN-1]    ='\0';
    strncpy(startDate[recCount],    date_fmt, STRLEN-1); startDate[recCount][STRLEN-1]   ='\0';
    recCount++;
    return 1;
}
int searchCountByPolicy(const char *key) { int c=0; for (int i=0;i<recCount;i++) if (containsIgnoreCase(policyNumber[i], key)) c++; return c; }
int searchCountByOwner (const char *key) { int c=0; for (int i=0;i<recCount;i++) if (containsIgnoreCase(ownerName[i],   key)) c++; return c; }
int searchCountByModel (const char *key) { int c=0; for (int i=0;i<recCount;i++) if (containsIgnoreCase(carModel[i],    key)) c++; return c; }

// 4) E2E cases
void e2e_add_save_load_search(void) {
    const char *FN = "e2e_policies.csv";
    printf("\n=== E2E-1: Add -> Save -> Reset -> Load -> Search ===\n");

    resetDB(); remove(FN);

    int ok1 = addPolicyDirect("A1",  "Ann",    "Civic", "2025-1-2");   // A001
    int ok2 = addPolicyDirect("B12", "Boon",   "City",  "2025-5-5");   // B012
    int ok3 = addPolicyDirect("A23", "Anucha", "Civic", "2025-9-9");   // A023
    assert_equal_int(1, ok1 && ok2 && ok3, "add 3 valid records");
    assert_equal_int(3, recCount, "recCount before saving");

    saveToCSVFile(FN);
    assert_equal_int(1, file_exists(FN), "CSV file created");
    assert_equal_int(3, count_csv_data_rows(FN), "CSV has 3 data rows");

    resetDB();
    assert_equal_int(0, recCount, "recCount reset");

    int loaded = loadFromCSVFile(FN);
    assert_equal_int(3, loaded, "loadFromCSVFile returns 3");
    assert_equal_int(3, recCount, "recCount after load");

    int idxA001 = findPolicyExact("A001");
    int idxB012 = findPolicyExact("B012");
    int idxA023 = findPolicyExact("A023");
    assert_equal_int(1, idxA001>=0 && idxB012>=0 && idxA023>=0, "all policies reloaded");

    assert_equal_str("Ann",          ownerName[idxA001], "A001 owner");
    assert_equal_str("Civic",        carModel[idxA001],  "A001 model");
    assert_equal_str("2025-01-02",   startDate[idxA001], "A001 date");

    assert_equal_int(2, searchCountByOwner("an"),    "search owner 'an' -> 2");
    assert_equal_int(2, searchCountByModel("Civic"), "search model 'Civic' -> 2");
    assert_equal_int(2, searchCountByPolicy("A0"),   "search policy prefix 'A0' -> 2");
}
void e2e_update_persist(void) {
    const char *FN = "e2e_policies.csv";
    printf("\n=== E2E-2: Update -> Save -> Reset -> Load ===\n");

    int ok = updatePolicyByNumber("A1", "Anne", "Civic RS", "2025-12-1");
    assert_equal_int(1, ok, "update A001 returns success");
    saveToCSVFile(FN);

    resetDB();
    loadFromCSVFile(FN);

    int idx = findPolicyExact("A001");
    assert_equal_int(1, idx >= 0,             "A001 exists after reload");
    assert_equal_str("Anne",      ownerName[idx], "A001 owner updated");
    assert_equal_str("Civic RS",  carModel[idx],  "A001 model updated");
    assert_equal_str("2025-12-01",startDate[idx], "A001 date updated");
}
void e2e_delete_persist(void) {
    const char *FN = "e2e_policies.csv";
    printf("\n=== E2E-3: Delete -> Save -> Reset -> Load ===\n");

    int ok = deletePolicyByNumber("B12");
    assert_equal_int(1, ok, "delete B012 returns success");
    saveToCSVFile(FN);

    resetDB();
    loadFromCSVFile(FN);

    assert_equal_int(-1, findPolicyExact("B012"), "B012 no longer exists");
    assert_equal_int(2, recCount, "recCount after delete and reload == 2");
}

// 5) main สำหรับรัน E2E
int main(void) {
    printf("===== RUNNING E2E TESTS (standalone) =====\n");

    T_TOTAL = T_FAIL = 0;
    e2e_add_save_load_search();
    e2e_update_persist();
    e2e_delete_persist();

    printf("\n----------------------------------\n");
    printf("E2E: %d check(s), %d failed\n", T_TOTAL, T_FAIL);
    printf("==================================\n");
    return (T_FAIL == 0) ? 0 : 1;
}