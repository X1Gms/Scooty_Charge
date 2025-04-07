#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void printLog(const char *format, ...);

void writeLogEntry(EventLog logEntry)
{
    FILE *logfile = fopen("logs.txt", "a");
    if (logfile == NULL)
    {
        printf("Error opening log file.\n");
        return;
    }

    // Format: [DD/MM/YYYY HH:MM:SS] Event Message
    fprintf(logfile, "[%02d/%02d/%04d %02d:%02d:%02d] %s\n",
            logEntry.timestamp.tm_mday,
            logEntry.timestamp.tm_mon + 1,     // tm_mon is 0-based
            logEntry.timestamp.tm_year + 1900, // tm_year is years since 1900
            logEntry.timestamp.tm_hour,
            logEntry.timestamp.tm_min,
            logEntry.timestamp.tm_sec,
            logEntry.eventType);

    fclose(logfile);
}

void printLog(const char *format, ...)
{
    EventLog entry;

    // Get current time
    time_t now = time(NULL);
    if (now == (time_t)-1)
    {
        printf("Error getting current time.\n");
        return;
    }

    // Convert to local time
    struct tm *localTime = localtime(&now);
    if (localTime == NULL)
    {
        printf("Error converting time.\n");
        return;
    }

    // Store the timestamp
    entry.timestamp = *localTime;

    // Prepare the formatted message using va_list
    va_list args;
    va_start(args, format);
    vsnprintf(entry.eventType, sizeof(entry.eventType), format, args);
    va_end(args);

    // Write the log entry
    writeLogEntry(entry);
}
// Function to read logs from file and display in GTK
void LoadLogsFromFile(GtkWidget *Logs_Txt)
{
    FILE *logfile = fopen("logs.txt", "r");
    if (logfile == NULL)
    {
        gtk_label_set_text(GTK_LABEL(Logs_Txt), "No logs available.");
        return;
    }

    char buffer[8192] = "";
    char line[256];
    while (fgets(line, sizeof(line), logfile) != NULL)
    {
        strcat(buffer, line);
    }
    fclose(logfile);

    gtk_label_set_text(GTK_LABEL(Logs_Txt), buffer);
}

void LogsContainer(GtkWidget *Logs)
{
    GtkWidget *Logs_C = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(Logs_C, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(Logs), Logs_C);

    GtkWidget *LogsB = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(LogsB, "LogsBox");
    gtk_widget_set_size_request(LogsB, 850, 625);
    gtk_box_append(GTK_BOX(Logs_C), LogsB);

    GtkWidget *Logs_Lbl = gtk_label_new("Logs");
    gtk_widget_set_name(Logs_Lbl, "LogsBox_Lbl");
    gtk_widget_set_valign(Logs_Lbl, GTK_ALIGN_START);
    gtk_widget_set_halign(Logs_Lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(LogsB), Logs_Lbl);

    GtkWidget *W_Logs = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(W_Logs, "W_Logs");
    gtk_widget_set_size_request(W_Logs, 500, 525);
    gtk_box_append(GTK_BOX(LogsB), W_Logs);

    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scroll_window, 500, 465);
    gtk_box_append(GTK_BOX(W_Logs), scroll_window);

    GtkWidget *Logs_Txt = gtk_label_new("");
    gtk_widget_set_name(Logs_Txt, "Logs_Txt");
    gtk_label_set_wrap(GTK_LABEL(Logs_Txt), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(Logs_Txt), PANGO_WRAP_WORD_CHAR);
    gtk_widget_set_halign(Logs_Txt, GTK_ALIGN_START);
    gtk_widget_set_valign(Logs_Txt, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(Logs_Txt), 0.0);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), Logs_Txt);
    LoadLogsFromFile(Logs_Txt);
}

void LoadLogs(GtkWidget *Logs)
{
    LoadNavBar(Logs);
    LogsContainer(Logs);
}