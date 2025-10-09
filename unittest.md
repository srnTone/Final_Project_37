Unit Tests — Car Insurance Management (C)

ชุดยูนิตเทสต์ครอบคลุมโหมด Add และ Search ของโปรเจกต์ โดยใช้ test runner ที่อยู่ในไฟล์ unittest.c เดียวกับโปรแกรมจริง

ไฟล์ที่ใช้

unittest.c — รวมโค้ดโปรแกรมหลัก + โค้ดยูนิตเทสต์ + test runner เอาไว้ในไฟล์เดียว

gcc -std=c11 -Wall -Wextra -DUNIT_TEST unittest.c -o unit_tests.exe

unit_tests.exe

ผลลัพธ์ที่คาดหวัง

[RUN] unit tests for addMode & searchMode

...

[OK] All tests passed.


รันโปรแกรมปกติ (ไม่ใช่โหมดเทสต์)

ใช้เมื่อต้องการเข้าเมนูโปรแกรมตามปกติ

gcc -std=c11 -Wall -Wextra unittest.c -o app

./app.exe
