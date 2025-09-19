#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

#pragma comment(lib, "dxva2.lib")

typedef struct {

    int pst_r;     // 100
    int pst_g;     // 100
    int pst_b;     // 100
    int nit_r;     // 100
    int nit_g;     // 55
    int nit_b;     // 10
    int dff_r;     // 85
    int dff_g;     // 100
    int dff_b;     // 100
    int now_br;    // 0
    // int dff_mode;  // 0
    BOOL is_mode;  // FALSE

} StoreStrc;

void quick_print(const char *data) {

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD count;
    WriteConsoleA(hConsole, data, strlen(data), &count, NULL);
    WriteConsoleA(hConsole, "\n", 1, &count, NULL);
}

void print_help(const char *name) {

    printf("\n%s h (print help)\n\n", name);
	printf("\tFormat:\t\t%s [MONITOR NO] - (Use '0' for Primary Monitor) [BRIGHTNESS %%] [RED %%] [GREEN %%] [BLUE %%]\n", name);
	printf("\tExample:\t%s 0 50 100 55 10\n", name);
}

// void set_rgb(HANDLE hMonitor, DWORD R, DWORD G, DWORD B) {
	
// 	SetMonitorRedGreenOrBlueGain(hMonitor, MC_BLUE_GAIN, B);
// 	SetMonitorRedGreenOrBlueGain(hMonitor, MC_GREEN_GAIN, G);
// 	SetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, R);
// }

int check_rgb(HANDLE hMonitor, DWORD R, DWORD G, DWORD B) {
	
    DWORD _R, miR, mxR; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miR, &_R, &mxR);
	DWORD _G, miG, mxG; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miG, &_G, &mxG);
	DWORD _B, miB, mxB; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miB, &_B, &mxB);
    
    if (R < miR || R > mxR || G < miG || G > mxG || B < miB || B > mxB) return 0;
    return 1;
}

int is_store(const wchar_t *f) {
    return (_waccess(f, 0) == 0);
}

wchar_t* get_store_path() {

    int MAXLEN = MAX_PATH * 4;
    wchar_t widePath[MAXLEN];
    
    if (GetModuleFileNameW(NULL, widePath, MAXLEN) == 0) return NULL;

    wchar_t *fixPath = wcsrchr(widePath, L'\\');
    if (fixPath == NULL) return NULL;

    size_t buffLen = fixPath - widePath;

    wchar_t *storePath = (wchar_t*)malloc((buffLen + 12) * sizeof(wchar_t));
    if (storePath == NULL) return NULL;

    wcsncpy(storePath, widePath, buffLen);
    storePath[buffLen] = L'\0';
    wcscat(storePath, L"\\store.bin");
    
    return storePath;
}


int main(int argc, char *argv[]) {

    if (argc <= 1) {

        quick_print("\nError: Insufficient arguments !");
        print_help(argv[0]);
        return 1;

    } else if (strcmp(argv[1], "h") == 0) {

        print_help(argv[0]);
        return 1;
    }

    wchar_t *storePath = get_store_path();
    size_t StrcSize = sizeof(StoreStrc);

    if (!storePath) {

        free(storePath);
        quick_print("\nError: Failed to get store.bin path !");
        return 1;
    }

    if (!is_store(storePath)) {

        StoreStrc writeData = {100, 100, 100, 100, 55, 10, 85, 100, 100, 0, FALSE};
        FILE *fp = _wfopen(storePath, L"wb");
        
        if (!fp) {

            free(storePath);
            quick_print("\nError: Failed to open store.bin file !");
            return 1;
        }
        fwrite(&writeData, StrcSize, 1, fp);
        fclose(fp);
    }

    StoreStrc readData;
    FILE *fp = _wfopen(storePath, L"rb");

    if (!fp) {

        free(storePath);
        quick_print("\nError: Failed to open store.bin file !");
        return 1;
    }
    fread(&readData, StrcSize, 1, fp);
    fclose(fp);
    
    

    // printf("V: %d%%\n", readData.pst_r);
    // printf("V: %d%%\n", readData.pst_g);
    // printf("V: %d%%\n", readData.pst_b);

    // printf("V: %d%%\n", readData.nit_r);
    // printf("V: %d%%\n", readData.nit_g);
    // printf("V: %d%%\n", readData.nit_b);

    // printf("V: %d%%\n", readData.dff_r);
    // printf("V: %d%%\n", readData.dff_g);
    // printf("V: %d%%\n", readData.dff_b);

    // printf("V: %d%%\n", readData.now_br);

    DWORD nM;

    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &nM)) {

        PHYSICAL_MONITOR* monitors = malloc(nM * sizeof(PHYSICAL_MONITOR));
        if (GetPhysicalMonitorsFromHMONITOR(hMonitor, nM, monitors)) {
            
            DWORD bri = atoi(argv[2]);

            if (bri > 100) bri = 100;
            if (bri < 0) bri = 0;

            // printf("V: %d%%\n", bri);
            
            if (argc >= 3 && strcmp(argv[1], "b") == 0 && isdigit(*argv[2])) {

                // printf("V: %d%%\n", bri);
                
                for (int i = 0; i < nM; i++) {

                    HANDLE pMon = monitors[i].hPhysicalMonitor;
                    if (check_rgb(pMon, readData.pst_r, readData.pst_g, readData.pst_b)) {

                        SetMonitorBrightness(pMon, bri);

                        SetMonitorRedGreenOrBlueGain(pMon, MC_RED_GAIN, readData.pst_r);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_GREEN_GAIN, readData.pst_g);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_BLUE_GAIN, readData.pst_b);

                        printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", i+1, bri, readData.pst_r, readData.pst_g, readData.pst_b);
                    } else printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", i+1, bri, readData.pst_r, readData.pst_g, readData.pst_b);
                }

                StoreStrc writeData = {

                    readData.pst_r,
                    readData.pst_g,
                    readData.pst_b,
                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    bri,
                    TRUE,
                };
                FILE *fp = _wfopen(storePath, L"wb");
                
                if (!fp) {

                    free(storePath);
                    quick_print("\nError: Failed to open store.bin file !");
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

            } else if (argc >= 3 && strcmp(argv[1], "c") == 0 && isdigit(*argv[2]) && isdigit(*argv[3]) && isdigit(*argv[4])) {

                DWORD vR = atoi(argv[2]);
                DWORD vG = atoi(argv[3]);
                DWORD vB = atoi(argv[4]);

                for (int i = 0; i < nM; i++) {

                    HANDLE pMon = monitors[i].hPhysicalMonitor;
                    if (check_rgb(pMon, vR, vG, vB)) {

                        // SetMonitorBrightness(pMon, bri);

                        printf("V: %d%%\n", readData.now_br);

                        SetMonitorRedGreenOrBlueGain(pMon, MC_RED_GAIN, vR);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_GREEN_GAIN, vG);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_BLUE_GAIN, vB);

                        printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, vR, vG, vB);
                    } else printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, vR, vG, vB);
                }

                StoreStrc writeData = {

                    vR,
                    vG,
                    vB,
                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    readData.now_br,
                    TRUE,
                };
                FILE *fp = _wfopen(storePath, L"wb");
                
                if (!fp) {

                    free(storePath);
                    quick_print("\nError: Failed to open store.bin file !");
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);
            }

            // for (int i = 0; i < (nM-1); i++) {

            //     HANDLE pMon = monitors[i].hPhysicalMonitor;
            //     if (check_rgb(pMon, vR, vG, vB)) {

            //         set_bright(pMon, Br);
            //         set_rgb(pMon, vR, vG, vB);
            //         printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", i, Br, vR, vG, vB);

            //     }
            // }



            // HANDLE pMon = monitors[vM].hPhysicalMonitor;
            // if (check_rgb(pMon, vR, vG, vB)) {

            //     set_bright(pMon, Br);
            //     set_rgb(pMon, vR, vG, vB);
            //     printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", vM, Br, vR, vG, vB);

            // } else {

            //     printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", vM, Br, vR, vG, vB);
            //     DestroyPhysicalMonitors(nM, monitors);
            //     free(monitors);

            //     return 1;
            // }









            DestroyPhysicalMonitors(nM, monitors);
        }
        free(monitors);


    }






    // if (argc >= 3 && strcmp(argv[1], "b") == 0 && isdigit(*argv[2])) {
        
    // }







    

    free(storePath);
    return 0;
}
