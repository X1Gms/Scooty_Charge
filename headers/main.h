#define ROWS 2
#define COLS 2

#include <gtk/gtk.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    GtkWidget *home_icon;
    GtkWidget *logs_icon;
    GtkWidget *home_text;
    GtkWidget *logs_text;
    GtkWidget *Home;
    GtkWidget *Logs;
    GtkWidget *USB;
    GtkWidget *Logs_C;
} Pagination;

typedef struct
{
    int number;        // battery percentage
    gboolean isLocked; // TRUE if locked, FALSE otherwise
} Battery;

typedef struct
{
    struct tm timestamp;
    char eventType[256];
} EventLog;

typedef struct
{
    int index;
    char portName[16];
} USBPort;

char batteryImgs[11][30] = {
    "./imgs/Percent/0p.png",
    "./imgs/Percent/10p.png",
    "./imgs/Percent/20p.png",
    "./imgs/Percent/30p.png",
    "./imgs/Percent/40p.png",
    "./imgs/Percent/50p.png",
    "./imgs/Percent/60p.png",
    "./imgs/Percent/70p.png",
    "./imgs/Percent/80p.png",
    "./imgs/Percent/90p.png",
    "./imgs/Percent/100p.png",
};

Battery batteryScooters[ROWS][COLS] = {
    {
        {0, FALSE},
        {0, FALSE},
    },
    {
        {0, FALSE},
        {0, FALSE},
    },
};

// Arrays para armazenar os ponteiros dos widgets que serão atualizados
GtkWidget *batteryImageWidgets[ROWS][COLS];
GtkWidget *batteryPercentageLabels[ROWS][COLS];
GtkWidget *statusLabels[ROWS][COLS]; // Usado na grade de status
GtkWidget *lockLabels[ROWS][COLS];
GtkWidget *lockedScootersLabel; // Ponteiro para o label que mostra o número de trotinetes "Locked"
int simulation_data[ROWS * COLS] = {0, 0, 0, 0};

GtkWidget *Home;
GtkWidget *Logs;
GtkWidget *USB;

Pagination *app_widgets;
HANDLE hSerial = INVALID_HANDLE_VALUE;
