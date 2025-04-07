#include "./headers/main.h"
#include "./src/sidebar.c"
#include "./src/series.c"
#include "./src/home.c"
#include "./src/logs.c"

// Function to load CSS
static void
load_css(void)
{
    GtkCssProvider *provider;
    GdkDisplay *display;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();

    gtk_style_context_add_provider_for_display(display,
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Load the CSS file
    GFile *css_file = g_file_new_for_path("style.css");
    const gchar *css_path = g_file_get_path(css_file);

    if (css_path)
    {
        gtk_css_provider_load_from_path(provider, css_path);
        g_free((gchar *)css_path);
    }
    else
    {
        g_printerr("Failed to load CSS file: Invalid path\n");
    }

    g_object_unref(css_file);
    g_object_unref(provider);
}

// Callback to exit the application
static void on_destroy(GtkWidget *widget, gpointer data)
{
    g_application_quit(G_APPLICATION(data));
}

static void activate(GtkApplication *app, gpointer user_data)
{
    // Load CSS
    load_css();

    // Create window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Scooty Charge");
    gtk_window_set_default_size(GTK_WINDOW(window), 1566, 830);
    gtk_widget_set_name(window, "window");

    // Prevent window resizing
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Create a container for the main layout
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    Home = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    Logs = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    USB = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_set_visible(GTK_WIDGET(Logs), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(USB), FALSE);

    // Load sidebar
    LoadSidebar(window, main_box, Home, Logs, USB);
    gtk_box_append(GTK_BOX(main_box), Home);
    gtk_box_append(GTK_BOX(main_box), Logs);
    gtk_box_append(GTK_BOX(main_box), USB);
    LoadHome(Home);
    LoadLogs(Logs);
    LoadComboUSB(USB);

    g_timeout_add_seconds(2, update_battery_simulation, NULL);
    // Show window
    gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.ScootyCharge", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}