#include <gtk/gtk.h>
#include <unistd.h>

void LoadComboUSB(GtkWidget *USB);
void writeSerialData(HANDLE hSerial, const char *data);
void reset_batteries();
void LoadLogs(GtkWidget *Logs);

static void
on_reset_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    writeSerialData(hSerial, "r");
    usleep(2000000);
    reset_batteries();
}

static void
on_home_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    Pagination *widgets = (Pagination *)user_data;

    // Change the images (for example, to "active" versions)
    gtk_image_set_from_file(GTK_IMAGE(widgets->home_icon), "./imgs/Home_En.png");
    gtk_image_set_from_file(GTK_IMAGE(widgets->logs_icon), "./imgs/Logs.png");

    // Change the widget names (used for CSS styling)
    gtk_widget_set_name(widgets->home_text, "Op_Enable");
    gtk_widget_set_name(widgets->logs_text, "Op_Normal");

    gtk_widget_set_visible(GTK_WIDGET(widgets->Home), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(widgets->USB), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(widgets->Logs), FALSE);
}

static void
on_logs_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    Pagination *widgets = (Pagination *)user_data;

    // Change icons and text styles
    gtk_image_set_from_file(GTK_IMAGE(widgets->home_icon), "./imgs/Home.png");
    gtk_image_set_from_file(GTK_IMAGE(widgets->logs_icon), "./imgs/Logs_En.png");
    gtk_widget_set_name(widgets->home_text, "Op_Normal");
    gtk_widget_set_name(widgets->logs_text, "Op_Enable");

    gtk_widget_set_visible(GTK_WIDGET(widgets->Home), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(widgets->USB), FALSE);

    // Destroy all children of widgets->Logs
    GtkWidget *child = gtk_widget_get_first_child(widgets->Logs);
    while (child != NULL)
    {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_widget_unparent(child);
        child = next;
    }

    gtk_widget_set_visible(GTK_WIDGET(widgets->Logs), TRUE);

    // Reload fresh content
    LoadLogs(widgets->Logs);
}

static void
on_usb_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    Pagination *widgets = (Pagination *)user_data;

    gtk_widget_set_visible(GTK_WIDGET(widgets->Home), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(widgets->USB), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(widgets->Logs), FALSE);
}

static void
on_exit_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_close(window);
}

static void on_refresh_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    GtkWidget *usb_container = GTK_WIDGET(user_data);
    if (!usb_container)
    {
        g_printerr("usb_container is NULL!\n");
        return;
    }

    g_print("Refresh clicked, reloading USB devices...\n");
    LoadComboUSB(usb_container);
}

void LoadNavBar(GtkWidget *Container)
{
    GtkWidget *Navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(Navbar, 1005, 130);
    gtk_box_set_homogeneous(GTK_BOX(Navbar), FALSE);
    gtk_box_append(GTK_BOX(Container), Navbar);

    // Spacer widget to push items to the right
    GtkWidget *spacer = gtk_label_new(NULL);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(Navbar), spacer);

    // Bluetooth Image
    GtkWidget *USB_Box = gtk_image_new_from_file("./imgs/USB.png");
    gtk_widget_set_size_request(USB_Box, 60, 60);
    gtk_widget_set_margin_end(USB_Box, 25);
    gtk_box_append(GTK_BOX(Navbar), USB_Box);

    GtkGesture *clickUSB = gtk_gesture_click_new();
    gtk_widget_add_controller(USB_Box, GTK_EVENT_CONTROLLER(clickUSB));

    g_signal_connect(clickUSB, "pressed", G_CALLBACK(on_usb_clicked), app_widgets);

    // Reset Image
    GtkWidget *Reset_Box = gtk_image_new_from_file("./imgs/Reset.png");
    gtk_widget_set_size_request(Reset_Box, 60, 60);
    gtk_widget_set_margin_end(Reset_Box, 25);
    gtk_box_append(GTK_BOX(Navbar), Reset_Box);

    GtkGesture *clickReset = gtk_gesture_click_new();
    gtk_widget_add_controller(Reset_Box, GTK_EVENT_CONTROLLER(clickReset));

    g_signal_connect(clickReset, "pressed", G_CALLBACK(on_reset_clicked), app_widgets);
}

void LoadResetBar(GtkWidget *Container)
{
    GtkWidget *Navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(Navbar, 1005, 110);
    gtk_box_set_homogeneous(GTK_BOX(Navbar), FALSE);
    gtk_box_append(GTK_BOX(Container), Navbar);

    // Spacer widget to push items to the right
    GtkWidget *spacer = gtk_label_new(NULL);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(Navbar), spacer);
    // Reset Image
    GtkWidget *Reset_Box = gtk_image_new_from_file("./imgs/Reset.png");
    gtk_widget_set_size_request(Reset_Box, 60, 60);
    gtk_widget_set_margin_end(Reset_Box, 25);
    gtk_box_append(GTK_BOX(Navbar), Reset_Box);

    GtkGesture *clickUSB = gtk_gesture_click_new();
    gtk_widget_add_controller(Reset_Box, GTK_EVENT_CONTROLLER(clickUSB));

    g_signal_connect(clickUSB, "pressed", G_CALLBACK(on_refresh_clicked), USB);
}

void LoadSidebar(GtkWidget *window, GtkWidget *main_box, GtkWidget *Home_C, GtkWidget *Logs_C, GtkWidget *USB_C)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(sidebar, "sidebar");
    gtk_box_append(GTK_BOX(main_box), sidebar);
    gtk_widget_set_size_request(sidebar, 275, -1); // -1 for flexible height

    GtkWidget *Scooty_Charge_Label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(Scooty_Charge_Label), "Scooty<span color='#71DACC'>Charge</span>");
    gtk_widget_set_name(Scooty_Charge_Label, "Scootylabel");

    GtkWidget *Hr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(Hr, 150, 5);
    gtk_widget_set_name(Hr, "HR");

    gtk_box_append(GTK_BOX(sidebar), Scooty_Charge_Label);
    gtk_box_append(GTK_BOX(sidebar), Hr);

    /* --- Home box --- */
    GtkWidget *Home = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(Home, 100, 0);
    gtk_widget_set_name(Home, "Op_Home");
    gtk_box_append(GTK_BOX(sidebar), Home);

    GtkWidget *Home_Icon = gtk_image_new_from_file("./imgs/Home_En.png");
    gtk_widget_set_size_request(Home_Icon, 50, 50);
    gtk_box_append(GTK_BOX(Home), Home_Icon);

    GtkWidget *Home_Text = gtk_label_new("Home");
    gtk_widget_set_name(Home_Text, "Op_Enable");
    gtk_box_append(GTK_BOX(Home), Home_Text);

    /* --- Logs box --- */
    GtkWidget *Logs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(Logs, 100, 0);
    gtk_widget_set_name(Logs, "Op_Logs");
    gtk_box_append(GTK_BOX(sidebar), Logs);

    GtkWidget *Logs_Icon = gtk_image_new_from_file("./imgs/Logs.png");
    gtk_widget_set_size_request(Logs_Icon, 50, 50);
    gtk_box_append(GTK_BOX(Logs), Logs_Icon);

    GtkWidget *Logs_Text = gtk_label_new("Logs");
    gtk_widget_set_name(Logs_Text, "Op_Normal");
    gtk_box_append(GTK_BOX(Logs), Logs_Text);

    // Allocate and initialize a structure to hold the widget pointers
    app_widgets = g_malloc(sizeof(Pagination));
    app_widgets->home_icon = Home_Icon;
    app_widgets->logs_icon = Logs_Icon;
    app_widgets->home_text = Home_Text;
    app_widgets->logs_text = Logs_Text;
    app_widgets->Home = Home_C;
    app_widgets->Logs = Logs_C;
    app_widgets->USB = USB_C;

    /* --- Attach click gesture to Home --- */
    GtkGesture *clickHome = gtk_gesture_click_new();
    gtk_widget_add_controller(Home, GTK_EVENT_CONTROLLER(clickHome));

    GtkGesture *clickLogs = gtk_gesture_click_new();
    gtk_widget_add_controller(Logs, GTK_EVENT_CONTROLLER(clickLogs));

    // Connect the "pressed" signal to update both the Home and Logs widgets.
    g_signal_connect(clickHome, "pressed", G_CALLBACK(on_home_clicked), app_widgets);

    g_signal_connect(clickLogs, "pressed", G_CALLBACK(on_logs_clicked), app_widgets);

    /* --- Exit Button --- */
    GtkWidget *s_Exit_Btn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top(s_Exit_Btn, 420);
    gtk_widget_set_halign(s_Exit_Btn, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(sidebar), s_Exit_Btn);

    GtkWidget *Exit_Btn = gtk_button_new();
    gtk_widget_set_size_request(Exit_Btn, 200, 70);
    gtk_widget_set_name(Exit_Btn, "Exit_Btn");

    GtkWidget *Exit_Btn_Text = gtk_label_new("Exit From App");
    gtk_widget_set_name(Exit_Btn_Text, "Exit_Btn_Text");
    g_object_set(Exit_Btn_Text, "halign", GTK_ALIGN_CENTER, NULL);
    gtk_button_set_child(GTK_BUTTON(Exit_Btn), Exit_Btn_Text);
    gtk_box_append(GTK_BOX(sidebar), Exit_Btn);

    g_signal_connect(Exit_Btn, "clicked", G_CALLBACK(on_exit_clicked), window);

    GtkWidget *Copyright = gtk_label_new("© Group 03 2024/2025");
    gtk_widget_set_name(Copyright, "Copyright");
    g_object_set(Copyright, "halign", GTK_ALIGN_CENTER, NULL);
    gtk_box_append(GTK_BOX(sidebar), Copyright);
}