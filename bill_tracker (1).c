#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_RECORDS 100
#define FILENAME "bills.csv"
#define SETTINGS_FILE "settings.csv"
#define ASKI(p, v) ((v) = readInt(p))
#define ASKF(p, v) ((v) = readFloat(p))

int readInt(const char *prompt) {
    int v, c;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &v) == 1) return v;
        while ((c = getchar()) != '\n' && c != EOF); // เคลียร์ค่าขยะที่กรอกผิดออกจาก buffer
        printf("กรุณากรอกตัวเลขเท่านั้น\n");
    }
}

float readFloat(const char *prompt) {
    float v; int c;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &v) == 1) return v;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("กรุณากรอกตัวเลขเท่านั้น\n");
    }
}
int useCustomRate = 0; // 0 = อัตรากรมฯ (ขั้นบันได), 1 = อัตราหอพัก (กำหนดเอง)
float customElecRate = 0, customWaterRate = 0;
typedef struct {
    int month, year;
    float elecOld, elecNew, waterOld, waterNew, elecCost, waterCost, roomRent, totalCost;
} BillRecord;
float tieredCost(float u, float t1, float t2, float p1, float p2, float p3) {
    if (u <= t1) return u * p1;
    if (u <= t2) return t1 * p1 + (u - t1) * p2;
    return t1 * p1 + (t2 - t1) * p2 + (u - t2) * p3;
}
float calculateElecCost(float u) { return tieredCost(u, 150, 400, 3.24, 4.22, 4.42); }
float calculateWaterCost(float u) { return tieredCost(u, 10, 30, 10.0, 15.0, 20.0); }
void addNewRecord(BillRecord r[], int *c); void saveToFile(BillRecord r[], int c); int loadFromFile(BillRecord r[]);
void showHistory(BillRecord r[], int c); void showStatistics(BillRecord r[], int c); void splitBill(BillRecord r[], int c);
void showMenu(); void setCustomRates(); void loadSettings(); void saveSettings();
int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    BillRecord records[MAX_RECORDS];
    int count = loadFromFile(records), choice;
    loadSettings();
    do {
        showMenu();
        ASKI("เลือกเมนู: ", choice);
        switch (choice) {
            case 1: addNewRecord(records, &count); saveToFile(records, count); break;
            case 2: showHistory(records, count); break;
            case 3: showStatistics(records, count); break;
            case 4: splitBill(records, count); break;
            case 5: setCustomRates(); break; // save settings ให้เองข้างในแล้ว
            case 6: printf("ขอบคุณที่ใช้งานโปรแกรม\n"); break;
            default: printf("กรุณาเลือกเมนูให้ถูกต้อง\n");
        }
        printf("\n");
    } while (choice != 6);
    return 0;
}
void showMenu() {
    printf("==== ระบบคำนวณค่าไฟ-น้ำรายเดือน ====\n");
    if (useCustomRate) printf("[โหมดปัจจุบัน: อัตราตามหอพัก -> ไฟ %.2f, น้ำ %.2f บาท/หน่วย]\n", customElecRate, customWaterRate);
    else printf("[โหมดปัจจุบัน: อัตรากรมไฟฟ้า/ประปา (ขั้นบันได)]\n");
    printf("1. บันทึกข้อมูลเดือนใหม่\n2. ดูประวัติย้อนหลัง\n3. ดูสถิติสรุป\n"
           "4. หารค่าใช้จ่ายกับเพื่อนร่วมห้อง\n5. ตั้งค่าอัตราค่าไฟ-น้ำ\n6. ออกจากโปรแกรม\n");
}
void addNewRecord(BillRecord records[], int *count) {
    if (*count >= MAX_RECORDS) { printf("บันทึกข้อมูลเต็มแล้ว ไม่สามารถเพิ่มได้\n"); return; }
    BillRecord r;
    ASKI("กรอกเดือน (1-12): ", r.month); ASKI("กรอกปี: ", r.year);
    ASKF("เลขมิเตอร์ไฟครั้งก่อน: ", r.elecOld); ASKF("เลขมิเตอร์ไฟครั้งล่าสุด: ", r.elecNew);
    ASKF("เลขมิเตอร์น้ำครั้งก่อน: ", r.waterOld); ASKF("เลขมิเตอร์น้ำครั้งล่าสุด: ", r.waterNew);
    ASKF("ค่าห้องเดือนนี้ (บาท): ", r.roomRent);
    float eu = r.elecNew - r.elecOld, wu = r.waterNew - r.waterOld;
    if (useCustomRate) { r.elecCost = eu * customElecRate; r.waterCost = wu * customWaterRate; }
    else { r.elecCost = calculateElecCost(eu); r.waterCost = calculateWaterCost(wu); }
    r.totalCost = r.elecCost + r.waterCost + r.roomRent;
    records[(*count)++] = r;
    printf("\n--- สรุปผล ---\nโหมดที่ใช้คำนวณ: %s\nหน่วยไฟที่ใช้: %.2f kWh\nหน่วยน้ำที่ใช้: %.2f ลบ.ม.\n"
           "ค่าไฟ: %.2f บาท\nค่าน้ำ: %.2f บาท\nค่าห้อง: %.2f บาท\nรวมทั้งหมด: %.2f บาท\nบันทึกข้อมูลเรียบร้อย\n",
           useCustomRate ? "อัตราตามหอพัก (กำหนดเอง)" : "อัตรากรมไฟฟ้า/ประปา (ขั้นบันได)", eu, wu, r.elecCost, r.waterCost, r.roomRent, r.totalCost);
}
void saveToFile(BillRecord records[], int count) {
    FILE *fp = fopen(FILENAME, "w");
    if (fp == NULL) { printf("ไม่สามารถเปิดไฟล์เพื่อบันทึกได้\n"); return; }
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", records[i].month, records[i].year,
                records[i].elecOld, records[i].elecNew, records[i].waterOld, records[i].waterNew,
                records[i].elecCost, records[i].waterCost, records[i].roomRent, records[i].totalCost);
    fclose(fp);
}
int loadFromFile(BillRecord records[]) {
    FILE *fp = fopen(FILENAME, "r");
    int count = 0;
    if (fp == NULL) return 0;
    while (count < MAX_RECORDS && fscanf(fp, "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f",
           &records[count].month, &records[count].year, &records[count].elecOld, &records[count].elecNew,
           &records[count].waterOld, &records[count].waterNew, &records[count].elecCost,
           &records[count].waterCost, &records[count].roomRent, &records[count].totalCost) == 10) count++;
    fclose(fp);
    return count;
}
void showHistory(BillRecord records[], int count) {
    if (count == 0) { printf("ยังไม่มีข้อมูลบันทึกไว้\n"); return; }
    printf("\n%-6s %-6s %-10s %-10s %-10s %-10s\n", "เดือน", "ปี", "ค่าไฟ", "ค่าน้ำ", "ค่าห้อง", "รวม");
    for (int i = 0; i < count; i++)
        printf("%-6d %-6d %-10.2f %-10.2f %-10.2f %-10.2f\n", records[i].month, records[i].year,
               records[i].elecCost, records[i].waterCost, records[i].roomRent, records[i].totalCost);
}
void showStatistics(BillRecord records[], int count) {
    if (count == 0) { printf("ยังไม่มีข้อมูลสำหรับคำนวณสถิติ\n"); return; }
    float sum = 0, maxCost = records[0].totalCost, minCost = records[0].totalCost;
    int maxM = records[0].month, maxY = records[0].year, minM = records[0].month, minY = records[0].year;
    for (int i = 0; i < count; i++) {
        sum += records[i].totalCost;
        if (records[i].totalCost > maxCost) { maxCost = records[i].totalCost; maxM = records[i].month; maxY = records[i].year; }
        if (records[i].totalCost < minCost) { minCost = records[i].totalCost; minM = records[i].month; minY = records[i].year; }
    }
    printf("\n--- สถิติสรุป ---\nค่าใช้จ่ายเฉลี่ยต่อเดือน: %.2f บาท\n", sum / count);
    printf("เดือนที่ใช้จ่ายสูงสุด: %d/%d (%.2f บาท)\n", maxM, maxY, maxCost);
    printf("เดือนที่ใช้จ่ายต่ำสุด: %d/%d (%.2f บาท)\n\nกราฟเปรียบเทียบค่าใช้จ่ายแต่ละเดือน:\n", minM, minY, minCost);
    for (int i = 0; i < count; i++) {
        printf("%d/%d: ", records[i].month, records[i].year);
        for (int j = 0; j < (int)(records[i].totalCost / 50); j++) printf("*");
        printf(" (%.2f บาท)\n", records[i].totalCost);
    }
}
void splitBill(BillRecord records[], int count) {
    if (count == 0) { printf("ยังไม่มีข้อมูลให้หารค่าใช้จ่าย\n"); return; }
    BillRecord latest = records[count - 1];
    int people;
    printf("บิลล่าสุดคือเดือน %d/%d รวม %.2f บาท\n", latest.month, latest.year, latest.totalCost);
    ASKI("จำนวนคนที่ร่วมหารค่าใช้จ่าย: ", people);
    if (people <= 0) { printf("จำนวนคนต้องมากกว่า 0\n"); return; }
    printf("แต่ละคนต้องจ่ายคนละ: %.2f บาท\n", latest.totalCost / people);
}
void setCustomRates() {
    int mode;
    printf("\n--- ตั้งค่าอัตราค่าไฟ-น้ำ ---\n1. อัตรากรมไฟฟ้า/ประปา (ขั้นบันได)\n2. อัตราตามหอพัก (กำหนดเอง)\n");
    ASKI("เลือกโหมด: ", mode);
    if (mode == 2) {
        useCustomRate = 1;
        ASKF("กรอกราคาค่าไฟที่หอพักคิด (บาท/หน่วย): ", customElecRate);
        ASKF("กรอกราคาค่าน้ำที่หอพักคิด (บาท/หน่วย): ", customWaterRate);
        printf("ตั้งค่าเรียบร้อย: ไฟ %.2f บาท/หน่วย, น้ำ %.2f บาท/หน่วย\n", customElecRate, customWaterRate);
    } else if (mode == 1) {
        useCustomRate = 0;
        printf("ตั้งค่าเรียบร้อย: ใช้อัตรากรมไฟฟ้า/ประปา (ขั้นบันได)\n");
    } else { printf("กรุณาเลือก 1 หรือ 2 เท่านั้น ไม่มีการเปลี่ยนแปลงค่าเดิม\n"); return; }
    saveSettings();
}
void loadSettings() {
    FILE *fp = fopen(SETTINGS_FILE, "r");
    if (fp == NULL || fscanf(fp, "%d,%f,%f", &useCustomRate, &customElecRate, &customWaterRate) != 3)
        { useCustomRate = 0; customElecRate = 0; customWaterRate = 0; }
    if (fp) fclose(fp);
}
void saveSettings() {
    FILE *fp = fopen(SETTINGS_FILE, "w");
    if (fp == NULL) { printf("ไม่สามารถบันทึกการตั้งค่าได้\n"); return; }
    fprintf(fp, "%d,%.2f,%.2f\n", useCustomRate, customElecRate, customWaterRate);
    fclose(fp);
}
