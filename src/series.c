#include <gtk/gtk.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Serial port connection function.
void update_status_grid();
void update_locked_scooters_label();
void printLog(const char *format, ...);

void reset_batteries()
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            batteryScooters[row][col].isLocked = FALSE;
            batteryScooters[row][col].number = 0;
        }
    }

    // Refresh the UI to reflect the reset
    update_status_grid();
    update_locked_scooters_label();

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            char labelText[40];

            // Update battery image
            snprintf(labelText, sizeof(labelText), "%s", batteryImgs[0]);
            gtk_image_set_from_file(GTK_IMAGE(batteryImageWidgets[row][col]), labelText);

            // Update battery percentage
            gtk_label_set_text(GTK_LABEL(batteryPercentageLabels[row][col]), "0%");
        }
    }
}

void writeSerialData(HANDLE hSerial, const char *data)
{
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, strlen(data), &bytesWritten, NULL))
    {
        printLog("Error writing to serial port\n");
    }
    else
    {
        printLog("Sent %lu bytes: %s\n", bytesWritten, data);
    }
}

HANDLE openSerialPort(const char *portName)
{
    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printLog("Error opening serial port %s\n", portName);
        return INVALID_HANDLE_VALUE;
    }
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        printLog("Error getting serial port parameters\n");
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        printLog("Error setting serial port parameters\n");
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    return hSerial;
}

// Forward declaration for LoadResetBar (assumed to be defined elsewhere)
void LoadResetBar(GtkWidget *USB);

// Structure to pass data to the click callback.
typedef struct
{
    char *portName;
    GtkWidget *status_label;
} DeviceData;

// Helper function to send an array of data over the serial port.
void sendData(HANDLE hSerial, const unsigned char *data, size_t len)
{
    DWORD bytesWritten;
    if (WriteFile(hSerial, data, len, &bytesWritten, NULL))
    {
        printLog("Data sent successfully: ");
        for (size_t i = 0; i < len; i++)
        {
            printf("%d ", data[i]);
        }
        printLog("\n");
    }
    else
    {
        printLog("Error writing data to serial port\n");
    }
}

// Helper function to read data from the serial port and update simulation_data with validation
void updateSimulationData(HANDLE hSerial)
{
    char buffer[64] = {0};
    DWORD bytesRead = 0;

    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Ensure null termination
        printLog("Dados recebidos (string): %s\n", buffer);

        // Remove spaces, newlines, or carriage returns at end of the string
        char *end = buffer + strlen(buffer) - 1;
        while (end > buffer && (isspace((unsigned char)*end) || *end == '\n' || *end == '\r'))
        {
            *end-- = '\0';
        }

        // Parse data separated by spaces or commas
        int values[ROWS * COLS] = {0};
        int count = 0;
        char *token = strtok(buffer, " ,");

        // Collect all tokens first to validate
        while (token != NULL && count < ROWS * COLS)
        {
            // Trim leading spaces
            while (isspace((unsigned char)*token))
                token++;

            // Trim trailing spaces
            char *t_end = token + strlen(token) - 1;
            while (t_end > token && isspace((unsigned char)*t_end))
                *t_end-- = '\0';

            // Validate each value is only 0 or 1
            int val = atoi(token);
            if (val != 0 && val != 1)
            {
                printLog("Invalid value received: %s (must be 0 or 1)\n", token);
                return; // Skip this update
            }

            values[count++] = val;
            token = strtok(NULL, " ,");
        }

        // Validate we have exactly 4 values
        if (count != ROWS * COLS)
        {
            printLog("Invalid data format: Expected %d values, got %d values\n", ROWS * COLS, count);
            return; // Skip this update
        }

        // Data is valid, update simulation_data
        for (int i = 0; i < ROWS * COLS; i++)
        {
            simulation_data[i] = values[i];
        }

        printLog("Dados simulados atualizados: %d %d %d %d\n",
                 simulation_data[0], simulation_data[1],
                 simulation_data[2], simulation_data[3]);
    }
    else
    {
        printLog("Erro a ler dados do serial\n");
    }
}

gboolean periodic_update(gpointer data)
{
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        updateSimulationData(hSerial);
        return TRUE; // Continua a chamar a cada 2000ms
    }

    return FALSE; // Termina o loop se a ligação for inválida
}

// Click callback: attempts to connect to the selected port, sends data, reads the reply,
// and updates the status label.
static void on_device_clicked(GtkGestureClick *gesture, int n_press,
                              double x, double y, gpointer user_data)
{
    DeviceData *ddata = (DeviceData *)user_data;
    printLog("Attempting to connect to %s\n", ddata->portName);
    HANDLE hSerial = openSerialPort(ddata->portName);
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        gtk_label_set_text(GTK_LABEL(ddata->status_label), "Connected");
        gtk_widget_set_name(ddata->status_label, "Connected");

        // Data to be sent: [1, 0, 1, 0]
        unsigned char data[] = {0, 0, 0, 0};
        // sendData(hSerial, data, sizeof(data));

        // Read the reply from ESP32 and update simulation_data
        updateSimulationData(hSerial);
        printLog("Connected to %s\n", ddata->portName);

        // Começa leitura periódica
        g_timeout_add(2000, periodic_update, NULL);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(ddata->status_label), "Failed to Connect");
        printLog("Failed to connect to %s\n", ddata->portName);
    }
}

// Function to create and load a device widget for a given COM port.
void LoadDevice(GtkWidget *content_area, const char *portName)
{
    // Create a horizontal box for this device.
    GtkWidget *device_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_size_request(device_box, 500, 100);
    gtk_widget_set_margin_bottom(device_box, 10);
    gtk_widget_set_halign(device_box, GTK_ALIGN_START);
    gtk_widget_set_name(device_box, "Device");
    gtk_box_append(GTK_BOX(content_area), device_box);

    // Create image container.
    GtkWidget *f_IMG = gtk_center_box_new();
    gtk_widget_set_size_request(f_IMG, 70, 80);
    gtk_widget_set_halign(f_IMG, GTK_ALIGN_START);
    gtk_widget_set_margin_end(f_IMG, 10);
    gtk_box_append(GTK_BOX(device_box), f_IMG);

    GtkWidget *usb_img = gtk_image_new_from_file("./imgs/USB.png");
    gtk_widget_set_size_request(usb_img, 50, 50);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(f_IMG), usb_img);

    // Create a vertical box for text labels.
    GtkWidget *f_TEXT = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_hexpand(f_TEXT, TRUE);
    gtk_widget_set_valign(f_TEXT, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(device_box), f_TEXT);

    // Device name label (including port name).
    char name_label_text[100];
    snprintf(name_label_text, sizeof(name_label_text), "Device (%s)", portName);
    GtkWidget *name_label = gtk_label_new(name_label_text);
    gtk_widget_set_name(name_label, "NameDevice");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(f_TEXT), name_label);

    // Status label, initially "Disconnected".
    GtkWidget *status_label = gtk_label_new("Disconnected");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_widget_set_name(status_label, "Disconnected");
    gtk_box_append(GTK_BOX(f_TEXT), status_label);

    // Prepare data for the click handler.
    DeviceData *ddata = malloc(sizeof(DeviceData));
    ddata->portName = strdup(portName); // Duplicate the port name string.
    ddata->status_label = status_label;

    // Create and attach a click gesture to the device box.
    GtkGesture *click = gtk_gesture_click_new();
    gtk_widget_add_controller(device_box, GTK_EVENT_CONTROLLER(click));
    g_signal_connect(click, "pressed", G_CALLBACK(on_device_clicked), ddata);
}

// Scans COM ports and creates a device widget for each available port.
void listSerialPorts(GtkWidget *content_area)
{
    printLog("Scanning for available serial ports...\n");
    for (int i = 1; i <= 256; i++)
    {
        char portName[16];
        snprintf(portName, sizeof(portName), "\\\\.\\COM%d", i);

        HANDLE hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0,
                                    NULL, OPEN_EXISTING, 0, NULL);
        if (hSerial != INVALID_HANDLE_VALUE)
        {
            LoadDevice(content_area, portName);
            printLog("Found %s\n", portName);
            CloseHandle(hSerial);
        }
    }
}

// Function that clears and reloads the USB container with detected devices.
void LoadComboUSB(GtkWidget *USB)
{
    // Remove any existing children from the USB container.
    GtkWidget *cUSB = gtk_widget_get_first_child(USB);
    while (cUSB)
    {
        GtkWidget *next = gtk_widget_get_next_sibling(cUSB);
        gtk_widget_unparent(cUSB);
        cUSB = next;
    }

    // Reset the USB container (implementation assumed).
    LoadResetBar(USB);

    // Create a vertical box to hold the content.
    GtkWidget *USBS = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(USBS, TRUE);
    gtk_widget_set_vexpand(USBS, TRUE);
    gtk_widget_set_valign(USBS, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(USBS, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(USB), USBS);

    // Create a container box for the scrolled area.
    cUSB = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(cUSB, 700, 600);
    gtk_widget_set_name(cUSB, "cUSB");
    gtk_box_append(GTK_BOX(USBS), cUSB);

    GtkWidget *NDevices;
    char label[100];

    // Create a scrolled window to hold the devices list.
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scroll_window, TRUE);
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_box_append(GTK_BOX(cUSB), scroll_window);

    // Create a box for the scrolled window's content.
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), content_area);

    // Scan for available serial ports and populate the content area.
    listSerialPorts(content_area);

    // Count the number of children (devices) in content_area.
    int count = 0;
    GtkWidget *child = gtk_widget_get_first_child(content_area);
    while (child)
    {
        count++;
        child = gtk_widget_get_next_sibling(child);
    }

    // Update the header with the number of devices found.
    snprintf(label, sizeof(label), "Number of Devices: %d", count);
    NDevices = gtk_label_new(label);
    gtk_widget_set_hexpand(NDevices, FALSE);
    gtk_widget_set_halign(NDevices, GTK_ALIGN_START);
    gtk_widget_set_name(NDevices, "NDevices");
    gtk_box_prepend(GTK_BOX(cUSB), NDevices);
}
