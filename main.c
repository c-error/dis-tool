#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

#pragma comment(lib, "dxva2.lib")

#define HELP_TEXT   "Command:\n\n" \
                    "\thelp\t\tprint help\n" \
                    "\toff\t\tturn off display\n" \
                    "\tnit\t\ttoggle night mode\n" \
                    "\trst\t\treset color to default\n" \
                    "\trgb <r% g% b%>\tadjust display color within 0-100% range\n" \
                    "\tbrit <v%>\tadjust display brightness within 0-100% range\n" \
                    "\tsetn <r% g% b%>\tset night light color within 0-100% range\n" \
                    "\tsetd <r% g% b%>\tset default color within 0-100% range\n" \
                    "\nAdjust brightness and color on a specific monitor:\n\n"

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

} StoreStrc;

void print_help(const char *e) {

    printf("\nUsage:\n\n\t%s <command>\n\n%s\t%s <moni-id> <bright%%> <r%%> <g%%> <b%%>\n\n", e, HELP_TEXT, e);
}

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

        printf("\nError: Insufficient arguments !");
        print_help(argv[0]);
        return 1;

    } else if (strcmp(argv[1], "help") == 0) {

        print_help(argv[0]);
        return 1;

    } else if (strcmp(argv[1], "off") == 0) {

        SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
        printf("\nDisplay: off\n");
        return 0;
    }

    wchar_t *storePath = get_store_path();
    size_t StrcSize = sizeof(StoreStrc);

    if (!storePath) {

        free(storePath);
        printf("\nError: Failed to get store.bin path !");
        return 1;
    }

    if (!is_store(storePath)) {

        StoreStrc writeData = {100, 100, 100, 100, 55, 10, 85, 100, 100, 0};
        FILE *fp = _wfopen(storePath, L"wb");
        
        if (!fp) {

            free(storePath);
            printf("\nError: Failed to open store.bin file !");
            return 1;
        }
        fwrite(&writeData, StrcSize, 1, fp);
        fclose(fp);
    }

    StoreStrc readData;
    FILE *fp = _wfopen(storePath, L"rb");

    if (!fp) {

        free(storePath);
        printf("\nError: Failed to open store.bin file !");
        return 1;
    }
    fread(&readData, StrcSize, 1, fp);
    fclose(fp);
    
    DWORD nM;

    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &nM)) {

        PHYSICAL_MONITOR* monitors = malloc(nM * sizeof(PHYSICAL_MONITOR));
        if (GetPhysicalMonitorsFromHMONITOR(hMonitor, nM, monitors)) {
            
            DWORD bri = atoi(argv[2]);
            if (bri > 100) bri = 100;
            if (bri < 0) bri = 0;

            if (argc >= 2 && strcmp(argv[1], "rst") == 0) {

                for (int i = 0; i < nM; i++) {

                    HANDLE pMon = monitors[i].hPhysicalMonitor;
                    if (check_rgb(pMon, readData.dff_r, readData.dff_g, readData.dff_b)) {

                        SetMonitorBrightness(pMon, readData.now_br);

                        SetMonitorRedGreenOrBlueGain(pMon, MC_RED_GAIN, readData.dff_r);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_GREEN_GAIN, readData.dff_g);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_BLUE_GAIN, readData.dff_b);

                        printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, readData.dff_r, readData.dff_g, readData.dff_b);
                    } else printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, readData.dff_r, readData.dff_g, readData.dff_b);
                }

                StoreStrc writeData = {

                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    readData.now_br,
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

            } else if (argc >= 5 && strcmp(argv[1], "setd") == 0 && isdigit(*argv[2]) && isdigit(*argv[3]) && isdigit(*argv[4])) {

                DWORD vR = atoi(argv[2]);
                DWORD vG = atoi(argv[3]);
                DWORD vB = atoi(argv[4]);

                StoreStrc writeData = {

                    readData.pst_r,
                    readData.pst_g,
                    readData.pst_b,
                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    vR,
                    vG,
                    vB,
                    readData.now_br,
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

                printf("\nSet Default: RGB(%d%%, %d%%, %d%%)\n", vR, vG, vB);

            } else if (argc >= 5 && strcmp(argv[1], "setn") == 0 && isdigit(*argv[2]) && isdigit(*argv[3]) && isdigit(*argv[4])) {

                DWORD vR = atoi(argv[2]);
                DWORD vG = atoi(argv[3]);
                DWORD vB = atoi(argv[4]);

                StoreStrc writeData = {

                    readData.pst_r,
                    readData.pst_g,
                    readData.pst_b,
                    vR,
                    vG,
                    vB,
                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    readData.now_br,
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

                printf("\nSet Night: RGB(%d%%, %d%%, %d%%)\n", vR, vG, vB);

            } else if (argc >= 2 && strcmp(argv[1], "nit") == 0) {

                for (int i = 0; i < nM; i++) {

                    HANDLE pMon = monitors[i].hPhysicalMonitor;
                    if (check_rgb(pMon, readData.nit_r, readData.nit_g, readData.nit_b)) {

                        SetMonitorRedGreenOrBlueGain(pMon, MC_RED_GAIN, readData.nit_r);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_GREEN_GAIN, readData.nit_g);
                        SetMonitorRedGreenOrBlueGain(pMon, MC_BLUE_GAIN, readData.nit_b);

                        printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, readData.nit_r, readData.nit_g, readData.nit_b);
                    } else printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", i+1, readData.now_br, readData.nit_r, readData.nit_g, readData.nit_b);
                }

                StoreStrc writeData = {

                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    readData.nit_r,
                    readData.nit_g,
                    readData.nit_b,
                    readData.dff_r,
                    readData.dff_g,
                    readData.dff_b,
                    readData.now_br,
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

            } else if (argc >= 3 && strcmp(argv[1], "brit") == 0 && isdigit(*argv[2])) {

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
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);

            } else if (argc >= 5 && strcmp(argv[1], "rgb") == 0 && isdigit(*argv[2]) && isdigit(*argv[3]) && isdigit(*argv[4])) {

                DWORD vR = atoi(argv[2]);
                DWORD vG = atoi(argv[3]);
                DWORD vB = atoi(argv[4]);

                for (int i = 0; i < nM; i++) {

                    HANDLE pMon = monitors[i].hPhysicalMonitor;
                    if (check_rgb(pMon, vR, vG, vB)) {

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
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");

                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);
            
            } else if (argc >= 6 && isdigit(*argv[1]) && isdigit(*argv[2]) && isdigit(*argv[3]) && isdigit(*argv[4]) && isdigit(*argv[5])) {

                DWORD Mo = atoi(argv[1]);
                DWORD Br = atoi(argv[2]);
                DWORD vR = atoi(argv[3]);
                DWORD vG = atoi(argv[4]);
                DWORD vB = atoi(argv[5]);

                HANDLE pMon = monitors[Mo-1].hPhysicalMonitor;
                if (check_rgb(pMon, vR, vG, vB)) {

                    SetMonitorBrightness(pMon, Br);

                    SetMonitorRedGreenOrBlueGain(pMon, MC_RED_GAIN, vR);
                    SetMonitorRedGreenOrBlueGain(pMon, MC_GREEN_GAIN, vG);
                    SetMonitorRedGreenOrBlueGain(pMon, MC_BLUE_GAIN, vB);

                    printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", Mo, Br, vR, vG, vB);
                } else printf("\nERROR: Out of Range ! monitor (%d), bright (%d%%), RGB(%d%%, %d%%, %d%%)\n", Mo, Br, vR, vG, vB);

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
                    Br,
                };

                FILE *fp = _wfopen(storePath, L"wb");
                if (!fp) {

                    free(storePath);
                    printf("\nError: Failed to open store.bin file !");
                    
                    DestroyPhysicalMonitors(nM, monitors);
                    free(monitors);
                    return 1;
                }
                fwrite(&writeData, StrcSize, 1, fp);
                fclose(fp);
            
            } else {

                printf("\nError: Insufficient arguments !");
                print_help(argv[0]);

                DestroyPhysicalMonitors(nM, monitors);

                free(monitors);
                free(storePath);

                return 1;
            }

            free(storePath);
            DestroyPhysicalMonitors(nM, monitors);
        }
        free(monitors);
    }

    return 0;
}
