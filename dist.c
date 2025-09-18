
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "dxva2.lib")

void print_on_error(const char *name) {

	printf("\nERROR: unknown value !\n\n");
	printf("\tFormat:\t\t%s [Monitor No.] [RED %%] [GREEN %%] [BLUE %%]\n", name);
	printf("\tExample:\t%s 0 100 15 15 (0 for Primary Monitor)\n", name);
}

// void set_bright(HANDLE hMonitor, int i) {
// 	SetMonitorBrightness(hMonitor, i);
// }

void value_set(HANDLE hMonitor, DWORD R, DWORD G, DWORD B) {
	
	SetMonitorRedGreenOrBlueGain(hMonitor, MC_BLUE_GAIN, B);
	SetMonitorRedGreenOrBlueGain(hMonitor, MC_GREEN_GAIN, G);
	SetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, R);
}

int check_range(HANDLE hMonitor, DWORD R, DWORD G, DWORD B) {
	
    DWORD _R, miR, mxR; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miR, &_R, &mxR);
	DWORD _G, miG, mxG; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miG, &_G, &mxG);
	DWORD _B, miB, mxB; GetMonitorRedGreenOrBlueGain(hMonitor, MC_RED_GAIN, &miB, &_B, &mxB);
    
    if (R < miR || R > mxR || G < miG || G > mxG || B < miB || B > mxB) return 0;
    return 1;
}

int main(int argc, char *argv[]) {

	if (argc < 6) { print_on_error(argv[0]); return 1; }

	DWORD vM, Br, vR, vG, vB;

	if (isdigit(*argv[1])) vM = atoi(argv[1]); else { print_on_error(argv[0]); return 1; }
	if (isdigit(*argv[2])) Br = atoi(argv[2]); else { print_on_error(argv[0]); return 1; }
	if (isdigit(*argv[3])) vR = atoi(argv[3]); else { print_on_error(argv[0]); return 1; }
	if (isdigit(*argv[4])) vG = atoi(argv[4]); else { print_on_error(argv[0]); return 1; }
	if (isdigit(*argv[5])) vB = atoi(argv[5]); else { print_on_error(argv[0]); return 1; }

    HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    DWORD numMonitors;
    PHYSICAL_MONITOR* monitors = NULL;
   
	if (Br > 100) Br = 100;
	if (Br < 0) Br = 0;

    if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numMonitors)) {
        
		monitors = malloc(numMonitors * sizeof(PHYSICAL_MONITOR));
        if (GetPhysicalMonitorsFromHMONITOR(hMonitor, numMonitors, monitors)) {
            
			HANDLE pMon = monitors[vM].hPhysicalMonitor;
            if (check_range(pMon, vR, vG, vB)) {

				SetMonitorBrightness(pMon, Br);
                value_set(pMon, vR, vG, vB);
				printf("\nMonitor: %d | Bright: %d%% | Color: RGB(%d%%, %d%%, %d%%)\n", vM, Br, vR, vG, vB);

            } else {
                printf("\nRGB values are out of range for this monitor (%d) !\n", vM);
            }

            DestroyPhysicalMonitors(numMonitors, monitors);
        }
        
        free(monitors);
    }

    return 0;
}






