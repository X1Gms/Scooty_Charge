#include <gtk/gtk.h>

void LoadStatusBox(GtkWidget *Home);
void LoadStatus(GtkWidget *container);
void LoadCharging(GtkWidget *container);
void LoadBatteries(GtkWidget *Status);
void GenerateBattery(GtkWidget *grid);
void LoadCircle(GtkWidget *Charging);
void LoadStatusGrid(GtkWidget *Charging);

void LoadStatusBox(GtkWidget *Home)
{
    // Create a container for better layout control
    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(container, 1005, 570);

    // Add the container to Home
    gtk_box_append(GTK_BOX(Home), container);

    LoadStatus(container);
    LoadCharging(container);
}

int count_locked_scooters()
{
    int count = 0;

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            if (batteryScooters[row][col].isLocked)
            {
                count++;
            }
        }
    }

    return count;
}

// Função auxiliar para determinar o texto do status conforme o nível e estado da bateria
static const char *get_status_text(Battery batt)
{
    if (batt.number == 10)
        return "Charged";
    else if (batt.isLocked)
        return "Charging";
    else if (batt.number > 0)
        return "Not Charged";
    else
        return "Shutdown";
}

// Função auxiliar para determinar a classe CSS (ou nome do widget) usada para estilizar o status
static const char *get_status_class(Battery batt)
{
    if (batt.number == 10)
        return "StatusContstats_charged";
    else if (batt.isLocked)
        return "StatusContstats_charging";
    else
        return "StatusContstats_not_charged";
}

void update_locked_scooters_label()
{
    if (lockedScootersLabel)
    {
        int n_Scooters = count_locked_scooters();
        char lbl_Scooters[16];
        snprintf(lbl_Scooters, sizeof(lbl_Scooters), "%d", n_Scooters);

        // Atualiza o texto do label
        gtk_label_set_text(GTK_LABEL(lockedScootersLabel), lbl_Scooters);
    }
}

// Função auxiliar para atualizar os labels da grade de status
void update_status_grid()
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            // Obtém o estado atual da bateria
            Battery batt = batteryScooters[row][col];

            // Atualiza os textos e classes CSS da grade de status
            const char *status_str = get_status_text(batt);
            const char *css_class = get_status_class(batt);
            gtk_label_set_text(GTK_LABEL(statusLabels[row][col]), status_str);
            gtk_widget_set_name(statusLabels[row][col], css_class);

            // Atualiza o estado de lock/unlock no grid principal
            const char *lockState = batt.isLocked ? "Locked" : "Unlocked";
            gtk_label_set_text(GTK_LABEL(lockLabels[row][col]), lockState);
            char lock_css[40];
            snprintf(lock_css, sizeof(lock_css), "c_Battery_%s", batt.isLocked ? "Locked" : "Unlocked");
            gtk_widget_set_name(lockLabels[row][col], lock_css);
        }
    }
}

gboolean update_battery_simulation(gpointer user_data)
{
    int index = 0;
    char labelText[40];

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            if (simulation_data[index] == 1)
            {
                batteryScooters[row][col].isLocked = TRUE;
                if (batteryScooters[row][col].number < 10)
                    batteryScooters[row][col].number++;
            }
            else
            {
                batteryScooters[row][col].isLocked = FALSE;
                batteryScooters[row][col].number = 0;
            }
            index++;

            snprintf(labelText, sizeof(labelText), "%s", batteryImgs[batteryScooters[row][col].number]);
            gtk_image_set_from_file(GTK_IMAGE(batteryImageWidgets[row][col]), labelText);

            snprintf(labelText, sizeof(labelText), "%d%%", batteryScooters[row][col].number * 10);
            gtk_label_set_text(GTK_LABEL(batteryPercentageLabels[row][col]), labelText);
        }
    }

    // Atualiza a grade de status e o número de trotinetes bloqueadas
    update_status_grid();
    update_locked_scooters_label();

    return TRUE; // Continua executando periodicamente
}

void LoadStatus(GtkWidget *container)
{
    // Create the spacer with a strict width
    GtkWidget *Status = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(Status, 70);
    gtk_widget_set_margin_start(Status, 30);
    gtk_widget_set_size_request(Status, 570, 630);
    gtk_widget_set_name(Status, "StatusContainer");

    gtk_box_append(GTK_BOX(container), Status);

    GtkWidget *ScootersLbl = gtk_label_new("Scooter Status");
    gtk_widget_set_name(ScootersLbl, "ScootersLbl");

    gtk_widget_set_halign(ScootersLbl, GTK_ALIGN_START);

    gtk_box_append(GTK_BOX(Status), ScootersLbl);
    LoadBatteries(Status);
}

void LoadBatteries(GtkWidget *Status)
{
    GtkWidget *grid;

    // Create a new grid widget
    grid = gtk_grid_new();
    gtk_widget_set_size_request(grid, 500, 400);

    // Set gaps between rows and columns
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_start(grid, 90);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 70); // Set horizontal gap between columns

    // Add the grid to the parent container (Status)
    gtk_box_append(GTK_BOX(Status), grid);
    GenerateBattery(grid);
}

// Função que gera o grid de baterias (imagens, porcentagens e labels de lock)
void GenerateBattery(GtkWidget *grid)
{
    GtkWidget *boxes[ROWS][COLS];
    char labelText[40];

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            // Cria o container para cada bateria
            boxes[row][col] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_add_css_class(boxes[row][col], "c_Battery");
            if (row > 0)
                gtk_widget_set_margin_top(boxes[row][col], 40);

            // Label para identificar a Scooter (ex.: "Scooter 01")
            snprintf(labelText, sizeof(labelText), "Scooter 0%d", row * COLS + col + 1);
            GtkWidget *scooterLabel = gtk_label_new(labelText);
            gtk_widget_add_css_class(scooterLabel, "c_Battery_lbl");
            gtk_widget_set_halign(scooterLabel, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(boxes[row][col]), scooterLabel);

            // Criação do container da bateria
            GtkWidget *batteryBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_margin_top(batteryBox, 10);
            gtk_widget_set_margin_bottom(batteryBox, 5);
            gtk_widget_set_halign(batteryBox, GTK_ALIGN_CENTER);
            gtk_widget_set_valign(batteryBox, GTK_ALIGN_CENTER);
            gtk_box_append(GTK_BOX(boxes[row][col]), batteryBox);

            // Overlay para empilhar a imagem e o rótulo do percentual
            GtkWidget *batteryOverlay = gtk_overlay_new();
            gtk_widget_add_css_class(batteryOverlay, "c_Battery_caps");
            gtk_widget_set_halign(batteryOverlay, GTK_ALIGN_CENTER);

            // Carrega a imagem (inicialmente com valor 0)
            snprintf(labelText, sizeof(labelText), "%s", batteryImgs[batteryScooters[row][col].number]);
            batteryImageWidgets[row][col] = gtk_image_new_from_file(labelText);
            gtk_widget_set_size_request(batteryImageWidgets[row][col], 90, 90);
            gtk_overlay_set_child(GTK_OVERLAY(batteryOverlay), batteryImageWidgets[row][col]);

            // Cria e posiciona o label do percentual
            snprintf(labelText, sizeof(labelText), "%d%%", batteryScooters[row][col].number * 10);
            batteryPercentageLabels[row][col] = gtk_label_new(labelText);
            gtk_widget_add_css_class(batteryPercentageLabels[row][col], "c_Battery_status_lbl");
            gtk_overlay_add_overlay(GTK_OVERLAY(batteryOverlay), batteryPercentageLabels[row][col]);
            gtk_widget_set_halign(batteryPercentageLabels[row][col], GTK_ALIGN_CENTER);
            gtk_widget_set_valign(batteryPercentageLabels[row][col], GTK_ALIGN_CENTER);

            gtk_box_append(GTK_BOX(batteryBox), batteryOverlay);

            // Label fixo "Status"
            GtkWidget *statusLabel = gtk_label_new("Status");
            gtk_widget_add_css_class(statusLabel, "c_Battery_status");
            gtk_box_append(GTK_BOX(boxes[row][col]), statusLabel);

            // Label que indica o estado de lock (Locked/Unlocked)
            const char *lockState = batteryScooters[row][col].isLocked ? "Locked" : "Unlocked";
            GtkWidget *lockLabel = gtk_label_new(lockState);
            snprintf(labelText, sizeof(labelText), "c_Battery_%s", batteryScooters[row][col].isLocked ? "Locked" : "Unlocked");
            gtk_widget_set_name(lockLabel, labelText);
            gtk_box_append(GTK_BOX(boxes[row][col]), lockLabel);

            // Armazena o ponteiro do label de lock para futura sincronização
            lockLabels[row][col] = lockLabel;

            // Anexa o container no grid principal
            gtk_grid_attach(GTK_GRID(grid), boxes[row][col], col, row, 1, 1);
        }
    }
}

void LoadCharging(GtkWidget *container)
{
    // Create the spacer with a strict width
    GtkWidget *Charging = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(Charging, 40);
    gtk_widget_set_size_request(Charging, 450, 630);
    gtk_widget_set_hexpand(Charging, FALSE); // Ensure it does not expand
    gtk_widget_set_vexpand(Charging, FALSE);
    gtk_widget_set_name(Charging, "StatusContainer");

    // Add the container to Home
    gtk_box_append(GTK_BOX(container), Charging);

    LoadCircle(Charging);
    LoadStatusGrid(Charging);
}

void LoadCircle(GtkWidget *Charging)
{
    GtkWidget *cCircle = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_halign(cCircle, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(cCircle, GTK_ALIGN_CENTER);

    gtk_box_append(GTK_BOX(Charging), cCircle);

    GtkWidget *lCircle = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_size_request(lCircle, 160, 160);

    gtk_widget_set_halign(lCircle, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(lCircle, GTK_ALIGN_CENTER);

    gtk_widget_set_name(lCircle, "Circle_Border");

    gtk_box_append(GTK_BOX(cCircle), lCircle);

    GtkWidget *slCircle = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_halign(slCircle, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(slCircle, GTK_ALIGN_CENTER);

    gtk_widget_set_size_request(slCircle, 135, 135);

    gtk_widget_set_hexpand(slCircle, TRUE);

    gtk_widget_set_name(slCircle, "Circle_Second_Border");

    gtk_box_append(GTK_BOX(lCircle), slCircle);

    GtkWidget *CItems = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_set_halign(CItems, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(CItems, GTK_ALIGN_CENTER);

    gtk_widget_set_hexpand(CItems, TRUE);

    gtk_box_append(GTK_BOX(slCircle), CItems);

    GtkWidget *Scooter_IMG = gtk_image_new_from_file("./imgs/scooter.png");

    gtk_widget_set_size_request(Scooter_IMG, 20, 20);

    gtk_box_append(GTK_BOX(CItems), Scooter_IMG);

    // Obtém o número inicial de trotinetes bloqueadas
    int n_Scooters = count_locked_scooters();
    char lbl_Scooters[16];
    snprintf(lbl_Scooters, sizeof(lbl_Scooters), "%d", n_Scooters);

    // Cria o label dinâmico para o número de trotinetes bloqueadas
    GtkWidget *Scooter_N = gtk_label_new(lbl_Scooters);
    gtk_widget_set_name(Scooter_N, "N_Scooters");
    gtk_box_append(GTK_BOX(CItems), Scooter_N);

    // Armazena o ponteiro do label para atualizações futuras
    lockedScootersLabel = Scooter_N;

    // Adiciona o label "Scooters" abaixo do número
    GtkWidget *scooters_label = gtk_label_new("Scooters");
    gtk_widget_set_name(scooters_label, "scooters_lbl");
    gtk_box_append(GTK_BOX(CItems), scooters_label);
}

// Função que carrega a grade de status (informações dinâmicas de "Charged", "Charging", "Shutdown")
void LoadStatusGrid(GtkWidget *charging)
{
    // Container principal centralizado em 'charging'
    GtkWidget *cstatus = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(cstatus, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(cstatus, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(charging), cstatus);

    // Para os 4 blocos (2x2)
    for (int i = 0; i < 4; i++)
    {
        int row = i / COLS;
        int col = i % COLS;

        // Container para cada status
        GtkWidget *status = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_margin_top(status, 30);
        gtk_widget_set_size_request(status, 300, 70);
        gtk_widget_set_name(status, "StatusCont");
        gtk_box_append(GTK_BOX(cstatus), status);
        gtk_widget_set_hexpand(status, TRUE);
        gtk_widget_set_halign(status, GTK_ALIGN_FILL);

        // Label identificando a scooter
        char scooter_text[64];
        snprintf(scooter_text, sizeof(scooter_text), "Scooter %02d", i + 1);
        GtkWidget *scooterLbl = gtk_label_new(scooter_text);
        gtk_widget_set_name(scooterLbl, "StatusContlbl");
        gtk_label_set_xalign(GTK_LABEL(scooterLbl), 0.0);

        // Obtém os dados atuais da bateria para definir o texto e a classe CSS
        Battery batt = batteryScooters[row][col];
        const char *status_str = get_status_text(batt);
        GtkWidget *statusLbl = gtk_label_new(status_str);
        gtk_widget_set_name(statusLbl, get_status_class(batt));
        gtk_label_set_xalign(GTK_LABEL(statusLbl), 0.0);
        gtk_widget_set_margin_start(statusLbl, 20);

        // Armazena o ponteiro para futuras atualizações
        statusLabels[row][col] = statusLbl;

        // Adiciona os labels ao container de status
        gtk_box_append(GTK_BOX(status), scooterLbl);
        gtk_box_append(GTK_BOX(status), statusLbl);
    }
}

void LoadHome(GtkWidget *Home)
{
    LoadNavBar(Home);
    LoadStatusBox(Home);
}