#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 35  // �޸�ҳ���С
#define CELL_SIZE 20

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point *body;
    int length;
    int direction;
} Snake;

typedef struct {
    char user_id[50];
    int score;
    char start_time[20];
} RankEntry;

Snake snake;
Point food;
Point super_food;
Point *obstacles;
Point portal1, portal2;
int obstacle_count;
GtkWidget *drawing_area;
GtkWidget *window;
GtkWidget *welcome_box;
GtkWidget *entry;
GtkWidget *info_label;
GtkWidget *user_id_label;
GtkWidget *score_label;
GtkWidget *level_label;
GtkWidget *difficulty_buttons[3];
GtkWidget *mode_buttons[2];
GtkWidget *play_button;
gboolean game_over = FALSE;
int level = 0;
int mode = 0; // ģʽ��0 = ����ģʽ��1 = ����ģʽ
guint timeout_id;
int timeout_interval;
char user_id[50] = "Anonymous user";
char start_time[20] = "";
int highest_score = 0;
gboolean super_food_active = FALSE;
guint timeout_id = 0; // ȷ����ʼ��Ϊ 0
gboolean accept_key_events = TRUE;
int snake_speed = 2;   // 1��ʾÿ��ˢ�¶��ƶ���2��ʾÿ����ˢ���ƶ�һ�Σ���������
int move_counter = 0;

// ��������
static void show_welcome_screen();
static void initialize_snake();
static void place_food();
static void place_portals();
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_timeout(gpointer data);
static void start_game(GtkWidget *widget, gpointer data);
static void restart_game(GtkWidget *widget, gpointer data);
static void continue_game(GtkWidget *widget, gpointer data);
static void return_to_welcome(GtkWidget *widget, gpointer data);
static void game_over_dialog();
static void save_rank_entry(const char *user_id, int score, const char *start_time);
static void show_leaderboard(GtkWidget *widget, gpointer data);
static void sort_and_save_rank_file();
static void select_level(GtkWidget *widget, gpointer data);
static void select_mode(GtkWidget *widget, gpointer data);
static gboolean on_draw_obstacle(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_draw_snake(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_draw_portal(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_draw_food(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_draw_super_food(GtkWidget *widget, cairo_t *cr, gpointer data);

// �������ü����¼�����
static void enable_key_events(GtkWidget *widget, gpointer data) {
    accept_key_events = TRUE;
}

static void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    // ��ջ���
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // �����ϰ���
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    for (int i = 0; i < obstacle_count; i++) {
        cairo_rectangle(cr, obstacles[i].x * CELL_SIZE, obstacles[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
    }
    cairo_fill(cr);

    // ������ͷ
    cairo_set_source_rgb(cr, 0.55, 0.65, 1); // ��ͷ��ɫ
    cairo_arc(cr, snake.body[0].x * CELL_SIZE + CELL_SIZE / 2, snake.body[0].y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2, 0, 2 * G_PI);
    cairo_fill(cr);

    // ��������
    cairo_set_source_rgb(cr, 0.65, 0.65, 0.7);
    for (int i = 1; i < snake.length; i++) {
        cairo_arc(cr, snake.body[i].x * CELL_SIZE + CELL_SIZE / 2, snake.body[i].y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2, 0, 2 * G_PI);
        cairo_fill(cr);
    }

    // ����ʳ��
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_arc(cr, food.x * CELL_SIZE + CELL_SIZE / 2, food.y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2, 0, 2 * G_PI);
    cairo_fill(cr);

    // ���Ƴ���ʳ��
    if (super_food.x != -1 && super_food.y != -1) {
        cairo_set_source_rgb(cr, 0.5, 0, 0.5); // �Ͻ�ɫ
        cairo_arc(cr, super_food.x * CELL_SIZE + CELL_SIZE / 2, super_food.y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2, 0, 2 * G_PI);
        cairo_fill(cr);
    }

    // ���ƴ�����
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_move_to(cr, portal1.x * CELL_SIZE + CELL_SIZE / 2, portal1.y * CELL_SIZE);
    cairo_line_to(cr, portal1.x * CELL_SIZE + CELL_SIZE, portal1.y * CELL_SIZE + CELL_SIZE / 2);
    cairo_line_to(cr, portal1.x * CELL_SIZE + CELL_SIZE / 2, portal1.y * CELL_SIZE + CELL_SIZE);
    cairo_line_to(cr, portal1.x * CELL_SIZE, portal1.y * CELL_SIZE + CELL_SIZE / 2);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_move_to(cr, portal2.x * CELL_SIZE + CELL_SIZE / 2, portal2.y * CELL_SIZE);
    cairo_line_to(cr, portal2.x * CELL_SIZE + CELL_SIZE, portal2.y * CELL_SIZE + CELL_SIZE / 2);
    cairo_line_to(cr, portal2.x * CELL_SIZE + CELL_SIZE / 2, portal2.y * CELL_SIZE + CELL_SIZE);
    cairo_line_to(cr, portal2.x * CELL_SIZE, portal2.y * CELL_SIZE + CELL_SIZE / 2);
    cairo_close_path(cr);
    cairo_fill(cr);

    // ����Χǽ�����ھ���ģʽ�£�
    if (mode == 1) {
        cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
        for (int i = 0; i < GRID_SIZE; i++) {
            cairo_rectangle(cr, i * CELL_SIZE, 0, CELL_SIZE, CELL_SIZE);
            cairo_rectangle(cr, i * CELL_SIZE, (GRID_SIZE - 1) * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            cairo_rectangle(cr, 0, i * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            cairo_rectangle(cr, (GRID_SIZE - 1) * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE);
        }
        cairo_fill(cr);
    }

    // ������Ϣ��ǩ
    char user_info[100];
    char score_info[100];
    char level_info[100];
    snprintf(user_info, sizeof(user_info), "User ID: %s", user_id);
    snprintf(score_info, sizeof(score_info), "Score: %d", snake.length - 3);
    snprintf(level_info, sizeof(level_info), "Difficulty: %d", level);
    gtk_label_set_text(GTK_LABEL(user_id_label), user_info);
    gtk_label_set_text(GTK_LABEL(score_label), score_info);
    gtk_label_set_text(GTK_LABEL(level_label), level_info);

    return FALSE;
}

static void place_food() {
    gboolean valid_position = FALSE;
    int is_super_food = (rand() % 100 < 20) ? 1 : 0; // 20%�ĸ������ɳ���ʳ��

    while (!valid_position) {
        if (is_super_food) {
            super_food.x = rand() % GRID_SIZE;
            super_food.y = rand() % GRID_SIZE;
        } else {
            food.x = rand() % GRID_SIZE;
            food.y = rand() % GRID_SIZE;
        }

        valid_position = TRUE;

        // ���ʳ���Ƿ����������
        for (int i = 0; i < snake.length; i++) {
            if ((is_super_food && snake.body[i].x == super_food.x && snake.body[i].y == super_food.y) ||
                (!is_super_food && snake.body[i].x == food.x && snake.body[i].y == food.y)) {
                valid_position = FALSE;
                break;
            }
        }

        // ���ʳ���Ƿ�����ϰ�����
        for (int i = 0; i < obstacle_count; i++) {
            if ((is_super_food && obstacles[i].x == super_food.x && obstacles[i].y == super_food.y) ||
                (!is_super_food && obstacles[i].x == food.x && obstacles[i].y == food.y)) {
                valid_position = FALSE;
                break;
            }
        }

        // ���ʳ���Ƿ����Χǽ�ϣ�����ģʽ��
        if (mode == 1 && (is_super_food && (super_food.x == 0 || super_food.x == GRID_SIZE - 1 || super_food.y == 0 || super_food.y == GRID_SIZE - 1) ||
                         (!is_super_food && (food.x == 0 || food.x == GRID_SIZE - 1 || food.y == 0 || food.y == GRID_SIZE - 1)))) {
            valid_position = FALSE;
        }
    }

    if (is_super_food) {
        food.x = -1; // ȷ����ͨʳ����ʧ
    } else {
        super_food.x = -1; // ȷ������ʳ����ʧ
    }
}

// ���ó���ʳ��
static void place_super_food() {
    if (!super_food_active) {
        super_food.x = rand() % GRID_SIZE;
        super_food.y = rand() % GRID_SIZE;
        super_food_active = TRUE;
    }
}

// ���ô�����
static void place_portals() {
    gboolean valid_position = FALSE;

    while (!valid_position) {
        portal1.x = rand() % GRID_SIZE;
        portal1.y = rand() % GRID_SIZE;
        portal2.x = rand() % GRID_SIZE;
        portal2.y = rand() % GRID_SIZE;

        valid_position = TRUE;

        // ��鴫�����Ƿ�����ϰ�����
        for (int i = 0; i < obstacle_count; i++) {
            if ((obstacles[i].x == portal1.x && obstacles[i].y == portal1.y) ||
                (obstacles[i].x == portal2.x && obstacles[i].y == portal2.y)) {
                valid_position = FALSE;
                break;
            }
        }

        // ��鴫�����Ƿ��ص�
        if (portal1.x == portal2.x && portal1.y == portal2.y) {
            valid_position = FALSE;
        }
    }
}

static void free_snake() {
    if (snake.body != NULL) {
        free(snake.body);
        snake.body = NULL;
    }
}

// ��ʼ����
static void initialize_snake() {
    free_snake(); // ���ͷ�֮ǰ���ڴ�
    snake.length = 3;
    snake.body = malloc(snake.length * sizeof(Point));
    if (snake.body == NULL) {
        g_print("Memory allocation failed for snake body\n");
        return;
    }
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = GRID_SIZE / 2;
        snake.body[i].y = GRID_SIZE / 2 + i;
    }
    snake.direction = 0; // ����
    super_food.x = -1;
	super_food.y = -1;
}

// ��ʼ���ϰ���
static void initialize_obstacles(int count) {
    obstacles = malloc(count * sizeof(Point));
    for (int i = 0; i < count; i++) {
        obstacles[i].x = rand() % GRID_SIZE;
        obstacles[i].y = rand() % GRID_SIZE;
    }
    obstacle_count = count;
}

//// �������а��¼
//static void save_rank_entry(const char *user_id, int score, const char *start_time) {
//    FILE *file = fopen("rank.txt", "a");
//    if (file == NULL) {
//        perror("Unable to open rank.txt");
//        return;
//    }
//    fprintf(file, "%s,%d,%s\n", user_id, score, start_time);
//    fclose(file);
//    sort_and_save_rank_file();
//}

// �������а��¼ 
static void save_rank_entry(const char *user_id, int score, const char *start_time) {
    char command[512];
    snprintf(command, sizeof(command), "./write_rank '%s' %d '%s'", user_id, score, start_time);
    int ret = system(command);
    if (ret == -1) {
        perror("Failed to execute write_rank");
    }
}


// ���򲢱������а��ļ�
static void sort_and_save_rank_file() {
    FILE *file = fopen("rank.txt", "r");
    if (file == NULL) {
        perror("Unable to open rank.txt");
        return;
    }

    RankEntry entries[100];
    int entry_count = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && entry_count < 100) {
        sscanf(line, "%49[^,],%d,%19[^\n]", entries[entry_count].user_id, &entries[entry_count].score, entries[entry_count].start_time);
        entry_count++;
    }
    fclose(file);

    // ����
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = i + 1; j < entry_count; j++) {
            if (entries[i].score < entries[j].score) {
                RankEntry temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }

    // ���������Ľ��
    file = fopen("rank.txt", "w");
    if (file == NULL) {
        perror("Unable to open rank.txt");
        return;
    }

    for (int i = 0; i < entry_count; i++) {
        fprintf(file, "%s,%d,%s\n", entries[i].user_id, entries[i].score, entries[i].start_time);
    }
    fclose(file);
}

// ��ʾ���а�
static void show_leaderboard(GtkWidget *widget, gpointer data) {
	accept_key_events = FALSE;  // ���ü����¼�����
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *tree_view;
    GtkListStore *list_store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeIter iter;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Leaderboard", GTK_WINDOW(window), flags, "Close", GTK_RESPONSE_CLOSE, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // �����б�洢
    list_store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);

    FILE *file = fopen("rank.txt", "r");
    if (file == NULL) {
        perror("Unable to open rank.txt");
    } else {
        char line[256];
        RankEntry entries[100];
        int entry_count = 0;

        while (fgets(line, sizeof(line), file) && entry_count < 100) {
            sscanf(line, "%49[^,],%d,%19[^\n]", entries[entry_count].user_id, &entries[entry_count].score, entries[entry_count].start_time);
            entry_count++;
        }
        fclose(file);

        // ��ʾǰ10��
        for (int i = 0; i < entry_count && i < 10; i++) {
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter,
                               0, i + 1,
                               1, entries[i].user_id,
                               2, entries[i].score,
                               3, entries[i].start_time,
                               -1);
        }
    }

    // ��������ͼ
    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    g_object_unref(list_store);

    // ����в����ж���
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new_with_attributes("Rank", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new_with_attributes("User ID", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new_with_attributes("Score", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new_with_attributes("Start Time", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    gtk_container_add(GTK_CONTAINER(content_area), tree_view);

    gtk_widget_show_all(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    
    // �ڶԻ�������ʱ�������ü����¼�����
    g_signal_connect(dialog, "destroy", G_CALLBACK(enable_key_events), NULL);
}

// ��Ϸ�����Ի���
static void game_over_dialog() {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *button_box;
    GtkWidget *button;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Game Over", GTK_WINDOW(window), flags, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("You lost! What would you like to do?");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(content_area), button_box);

    button = gtk_button_new_with_label("Continue");
    g_signal_connect(button, "clicked", G_CALLBACK(continue_game), dialog);
    gtk_container_add(GTK_CONTAINER(button_box), button);

    button = gtk_button_new_with_label("Restart");
    g_signal_connect(button, "clicked", G_CALLBACK(restart_game), dialog);
    gtk_container_add(GTK_CONTAINER(button_box), button);

    button = gtk_button_new_with_label("Return to Welcome");
    g_signal_connect(button, "clicked", G_CALLBACK(return_to_welcome), dialog);
    gtk_container_add(GTK_CONTAINER(button_box), button);

    gtk_widget_show_all(dialog);

    if (snake.length - 3 > highest_score) {
        highest_score = snake.length - 3;
    }
}

// ��Ϸѭ��
static gboolean on_timeout(gpointer data) {
    if (game_over) {
        game_over_dialog();
        return FALSE;
        
    }
    
    // ���Ӽ�����
    // move_counter++;
    	
    // ȷ�� drawing_area ����Ч��
    if (!GTK_IS_WIDGET(drawing_area)) {
        g_print("Error: drawing_area is not a valid widget\n");
        return FALSE;
    }
    
 	// ֻ�е��������ﵽ�ߵ��ٶ�ʱ���ƶ���
    // if (move_counter >= snake_speed) {
    //    move_counter = 0;

    // �ƶ�����
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }

    // �ƶ���ͷ
    switch (snake.direction) {
        case 0: snake.body[0].y -= 1; break; // ����
        case 1: snake.body[0].x += 1; break; // ����
        case 2: snake.body[0].y += 1; break; // ����
        case 3: snake.body[0].x -= 1; break; // ����
    }

    // ���߽紫�ͣ���������ģʽ�£�
    if (mode == 0) {
        if (snake.body[0].x < 0) snake.body[0].x = GRID_SIZE - 1;
        if (snake.body[0].x >= GRID_SIZE) snake.body[0].x = 0;
        if (snake.body[0].y < 0) snake.body[0].y = GRID_SIZE - 1;
        if (snake.body[0].y >= GRID_SIZE) snake.body[0].y = 0;
    } else {
        if (snake.body[0].x < 0 || snake.body[0].x >= GRID_SIZE || snake.body[0].y < 0 || snake.body[0].y >= GRID_SIZE) {
            game_over = TRUE;
        }
    }

    // ���ʳ����ײ
    if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
        snake.length++;
        snake.body = realloc(snake.body, snake.length * sizeof(Point));
        if (!snake.body) {
            g_print("Memory allocation failed for snake body\n");
            return FALSE;
        }
        snake.body[snake.length - 1] = snake.body[snake.length - 2];
        place_food();
    }

    // ��鳬��ʳ����ײ
    if (super_food.x != -1 && super_food.y != -1 && snake.body[0].x == super_food.x && snake.body[0].y == super_food.y) {
        int old_length = snake.length;
        snake.length += 5;
        snake.body = realloc(snake.body, snake.length * sizeof(Point));

        // ȷ���������Ĳ���λ�ú���
        for (int i = old_length; i < snake.length; i++) {
            snake.body[i] = snake.body[i - 1];
        }

        super_food.x = -1;
        super_food.y = -1;
        place_food();
    }

    // ��鴫������ײ
    if (snake.body[0].x == portal1.x && snake.body[0].y == portal1.y) {
        snake.body[0].x = portal2.x;
        snake.body[0].y = portal2.y;
    } else if (snake.body[0].x == portal2.x && snake.body[0].y == portal2.y) {
        snake.body[0].x = portal1.x;
        snake.body[0].y = portal1.y;
    }

    // ����ϰ�����ײ
    for (int i = 0; i < obstacle_count; i++) {
        if (snake.body[0].x == obstacles[i].x && snake.body[0].y == obstacles[i].y) {
            game_over = TRUE;
        }
    }

    // ���������ײ
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y) {
            game_over = TRUE;
        }
    }

    if (drawing_area && GTK_IS_WIDGET(drawing_area)) {
        gtk_widget_queue_draw(drawing_area);
    } else {
        g_print("Error: drawing_area is not a valid widget\n");
    }
	// }

    return TRUE;
}

// ��������¼�
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	if (!accept_key_events) {
        return FALSE;
    }
    if (entry && GTK_IS_WIDGET(entry) && gtk_widget_has_focus(entry)) {
        // ���������н��㣬�򲻴������������¼�
        return FALSE;
    }
    
    // g_print("Key pressed: %d\n", event->keyval);
    // g_print("Drawing area address in on_key_press: %p\n", (void *)drawing_area);
    
    if (!GTK_IS_WIDGET(drawing_area)) {
        g_print("Error: drawing_area is not a valid widget\n");
        return FALSE;
    }    
    
    if (snake.body == NULL) {
        g_print("Error: snake.body is NULL\n");
        return FALSE;
    }

    switch (event->keyval) {
        case GDK_KEY_Up:
            if (snake.direction != 2) snake.direction = 0; // ����
            break;
        case GDK_KEY_Right:
            if (snake.direction != 3) snake.direction = 1; // ����
            break;
        case GDK_KEY_Down:
            if (snake.direction != 0) snake.direction = 2; // ����
            break;
        case GDK_KEY_Left:
            if (snake.direction != 1) snake.direction = 3; // ����
            break;
        default:
            break;
    }
    
    // ȷ�� drawing_area ����Ч��
    if (snake.direction != -1) {
        // ȷ�� drawing_area ����Ч��
        // g_print("Drawing area address in on_key_press: %p\n", drawing_area);
        if (drawing_area && GTK_IS_WIDGET(drawing_area)) {
            gtk_widget_queue_draw(drawing_area);
        } else {
            g_print("Error: drawing_area is not a valid widget\n");
        }
    }

    return TRUE;
}

// ��ʼ��Ϸ
static void start_game(GtkWidget *widget, gpointer data) {
    int obstacle_count;
    if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }

    switch (level) {
        case 1:
        	//snake_speed = 15;
            obstacle_count = 10;
            timeout_interval = 160;
            break;
        case 2:
        	//snake_speed = 10;
            obstacle_count = 15;
            timeout_interval = 120;
            break;
        case 3:
        	//snake_speed = 5;
            obstacle_count = 20;
            timeout_interval = 80;
            break;
        default:
        	//snake_speed = 10;
            obstacle_count = 15;
            timeout_interval = 120;
    }

    const char *user_input = gtk_entry_get_text(GTK_ENTRY(entry));
    if (user_input && strlen(user_input) > 0) {
        snprintf(user_id, sizeof(user_id), "%s", user_input);
    } else {
        snprintf(user_id, sizeof(user_id), "Anonymous user");
    }

    initialize_snake();
    initialize_obstacles(obstacle_count);
    place_food();
    place_super_food();
    place_portals();
    game_over = FALSE;

    // ���ٻ�ӭ���沢��ʾ��Ϸ����
    if (welcome_box) {
        gtk_widget_destroy(welcome_box);
        welcome_box = NULL;
    }

    // ������ͼ�������Ϣ��ǩ
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    user_id_label = gtk_label_new(NULL);
    score_label = gtk_label_new(NULL);
    level_label = gtk_label_new(NULL);

    gtk_label_set_xalign(GTK_LABEL(user_id_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(score_label), 0.5);
    gtk_label_set_xalign(GTK_LABEL(level_label), 1.0);

    gtk_widget_set_size_request(user_id_label, 150, -1);  // �̶����
    gtk_widget_set_size_request(score_label, 150, -1);    // �̶����
    gtk_widget_set_size_request(level_label, 150, -1);    // �̶����

    gtk_box_pack_start(GTK_BOX(hbox), user_id_label, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), score_label, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), level_label, TRUE, TRUE, 20);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    // ȷ�����ڽ�����һ���Ӳ���
    GList *children = gtk_container_get_children(GTK_CONTAINER(window));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);

    g_print("Drawing area address in start_game: %p\n", (void *)drawing_area);

    // �����״ο�ʼ��Ϸʱ��¼��Ϸ��ʼʱ��
    if (start_time[0] == '\0') {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        strftime(start_time, sizeof(start_time), "%Y-%m-%d %H:%M:%S", tm_info);
    }
	timeout_id = g_timeout_add(timeout_interval, on_timeout, NULL);
    g_timeout_add(100, (GSourceFunc)gtk_widget_queue_draw, drawing_area);
}

// �� entry �ϰ��س���ʱ������ OK ��ť�ĵ���ź�
static void on_entry_activate(GtkWidget *entry, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

// ������Ϸ
static void continue_game(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    GtkWidget *ok_button;
    GtkWidget *error_label;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Enter Password", GTK_WINDOW(window), flags, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    // ��Ӵ�����ʾ��ǩ
    error_label = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(content_area), error_label);

    // ��ȡ OK ��ť������ΪĬ�ϰ�ť
    ok_button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_widget_set_can_default(ok_button, TRUE);
    gtk_widget_grab_default(ok_button);

    // ���ӻس����¼��� OK ��ť�ļ����ź�
    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate), dialog);

    gtk_widget_show_all(dialog);

    while (1) {
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_OK) {
            const char *password = gtk_entry_get_text(GTK_ENTRY(entry));
            if (strcmp(password, "2021014207") == 0) {
                gtk_widget_destroy(dialog);
                gtk_widget_destroy(GTK_WIDGET(data));
                game_over = FALSE;
                timeout_id = g_timeout_add(timeout_interval, on_timeout, NULL);
                return;
            } else {
                // �û�����������ʱ���������ı���ɫ����Ϊ��ɫ������ʾ������ʾ
                GdkRGBA red;
                gdk_rgba_parse(&red, "red");
                gtk_widget_override_background_color(entry, GTK_STATE_FLAG_NORMAL, &red);
                gtk_label_set_text(GTK_LABEL(error_label), "Incorrect Password! Please try again.");
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
    }
}


// ���¿�ʼ��Ϸ
static void restart_game(GtkWidget *widget, gpointer data) {
	if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
    
    if (snake.length - 3 > highest_score) {
        highest_score = snake.length - 3;
    }
    
	free_snake();
    save_rank_entry(user_id, highest_score, start_time);
    highest_score = 0;
    start_time[0] = '\0';  // ���ÿ�ʼʱ��
    gtk_widget_destroy(GTK_WIDGET(data));
    initialize_snake();
    initialize_obstacles(obstacle_count);
    place_food();
    place_portals();
    game_over = FALSE;
    timeout_id = g_timeout_add(timeout_interval, on_timeout, NULL);
    gtk_widget_queue_draw(drawing_area);
    
    // �����״ο�ʼ��Ϸʱ��¼��Ϸ��ʼʱ��
    if (start_time[0] == '\0') {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        strftime(start_time, sizeof(start_time), "%Y-%m-%d %H:%M:%S", tm_info);
    }
}

// ���ص���ӭ����
static void return_to_welcome(GtkWidget *widget, gpointer data) {
    accept_key_events = FALSE;  // ���ü����¼�����
    if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
    if (snake.length - 3 > highest_score) {
        highest_score = snake.length - 3;
    }
    save_rank_entry(user_id, highest_score, start_time);
    highest_score = 0;
    start_time[0] = '\0';  // ���ÿ�ʼʱ��
    gtk_widget_destroy(GTK_WIDGET(data));

    // ȷ�����ڽ�����һ���Ӳ���
    GList *children = gtk_container_get_children(GTK_CONTAINER(window));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        if (GTK_IS_WIDGET(drawing_area) && iter->data == drawing_area) {
            g_print("Error: drawing_area is not a valid widget\n");
            drawing_area = NULL;
        }
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    drawing_area = NULL; // ���ָ��

    show_welcome_screen();
    
    accept_key_events = TRUE;  // �������ü����¼�����
}

// ѡ���Ѷ�
static void select_level(GtkWidget *widget, gpointer data) {
    level = GPOINTER_TO_INT(data);
    for (int i = 0; i < 3; i++) {
        if (i == level - 1) {
            gtk_widget_set_name(difficulty_buttons[i], "selected");
        } else {
            gtk_widget_set_name(difficulty_buttons[i], "normal");
        }
    }
}

// ѡ��ģʽ
static void select_mode(GtkWidget *widget, gpointer data) {
    mode = GPOINTER_TO_INT(data);
    for (int i = 0; i < 2; i++) {
        if (i == mode) {
            gtk_widget_set_name(mode_buttons[i], "selected");
        } else {
            gtk_widget_set_name(mode_buttons[i], "normal");
        }
    }
}

// ��ʾ��ӭ����
static void show_welcome_screen() {
    GtkWidget *label;
    GtkWidget *button_box;
    GtkWidget *button;
    GtkWidget *vbox;

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<span font='24'>Welcome to Snake Game!</span>");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

    label = gtk_label_new("Enter your game ID");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Anonymous user");
    gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 10);

    label = gtk_label_new("Select difficulty");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_spacing(GTK_BOX(button_box), 10);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 10);

    button = gtk_button_new_with_label("Easy");
    difficulty_buttons[0] = button;
    g_signal_connect(button, "clicked", G_CALLBACK(select_level), GINT_TO_POINTER(1));
    gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 10);

    button = gtk_button_new_with_label("Medium");
    difficulty_buttons[1] = button;
    g_signal_connect(button, "clicked", G_CALLBACK(select_level), GINT_TO_POINTER(2));
    gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 10);

    button = gtk_button_new_with_label("Hard");
    difficulty_buttons[2] = button;
    g_signal_connect(button, "clicked", G_CALLBACK(select_level), GINT_TO_POINTER(3));
    gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 10);

    label = gtk_label_new("Select mode");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_spacing(GTK_BOX(button_box), 10);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 10);

    button = gtk_button_new_with_label("Free Mode");
    mode_buttons[0] = button;
    g_signal_connect(button, "clicked", G_CALLBACK(select_mode), GINT_TO_POINTER(0));
    gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 10);

    button = gtk_button_new_with_label("Classic Mode");
    mode_buttons[1] = button;
    g_signal_connect(button, "clicked", G_CALLBACK(select_mode), GINT_TO_POINTER(1));
    gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 10);

    button = gtk_button_new_with_label("PLAY");
    play_button = button;
    g_signal_connect(button, "clicked", G_CALLBACK(start_game), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);

    button = gtk_button_new_with_label("Show Leaderboard");
    g_signal_connect(button, "clicked", G_CALLBACK(show_leaderboard), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);

    // ���˵��
    GtkWidget *explanation_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(explanation_box), TRUE);

    GtkWidget *hbox;

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *obstacle_label = gtk_label_new("Obstacle: ");
    GtkWidget *obstacle_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(obstacle_area, 20, 20);
    g_signal_connect(obstacle_area, "draw", G_CALLBACK(on_draw_obstacle), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), obstacle_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), obstacle_area, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(explanation_box), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *snake_label = gtk_label_new("Snake: ");
    GtkWidget *snake_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(snake_area, 20, 20);
    g_signal_connect(snake_area, "draw", G_CALLBACK(on_draw_snake), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), snake_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), snake_area, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(explanation_box), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *portal_label = gtk_label_new("Portal: ");
    GtkWidget *portal_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(portal_area, 20, 20);
    g_signal_connect(portal_area, "draw", G_CALLBACK(on_draw_portal), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), portal_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), portal_area, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(explanation_box), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *food_label = gtk_label_new("Food: ");
    GtkWidget *food_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(food_area, 20, 20);
    g_signal_connect(food_area, "draw", G_CALLBACK(on_draw_food), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), food_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), food_area, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(explanation_box), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *super_food_label = gtk_label_new("Super Food: ");
    GtkWidget *super_food_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(super_food_area, 20, 20);
    g_signal_connect(super_food_area, "draw", G_CALLBACK(on_draw_super_food), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), super_food_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), super_food_area, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(explanation_box), hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), explanation_box, FALSE, FALSE, 10);

    label = gtk_label_new("PS: Superfoods can score 5 points! The chance to refresh is 20%!");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    
    label = gtk_label_new("Author: Muchenxixi_");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);

    gtk_widget_show_all(window);
}

// ��� CSS ��ʽ
static void add_css_style() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "#selected { background-color: #ffffff; color: #000000; font-weight: 500; font-size:18px; } "
        "#normal { background-color: #eeeeee; color: #000000; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static gboolean on_draw_obstacle(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_rectangle(cr, 0, 0, 20, 20);
    cairo_fill(cr);
    return FALSE;
}

static gboolean on_draw_snake(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 0.65, 0.65, 0.7);
    cairo_arc(cr, 10, 10, 10, 0, 2 * G_PI);
    cairo_fill(cr);
    return FALSE;
}

static gboolean on_draw_portal(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 0, 1, 0); // ��ɫ
    cairo_move_to(cr, 10, 0);
    cairo_line_to(cr, 20, 10);
    cairo_line_to(cr, 10, 20);
    cairo_line_to(cr, 0, 10);
    cairo_close_path(cr);
    cairo_fill(cr);
    return FALSE;
}

static gboolean on_draw_food(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 1, 0, 0); // ��ɫ
    cairo_arc(cr, 10, 10, 10, 0, 2 * G_PI);
    cairo_fill(cr);
    return FALSE;
}
static gboolean on_draw_super_food(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgb(cr, 0.5, 0, 0.5); // �Ͻ�ɫ
    cairo_arc(cr, 10, 10, 10, 0, 2 * G_PI);
    cairo_fill(cr);
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL));

    // ��������
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Snake Game");
    gtk_window_set_default_size(GTK_WINDOW(window), GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // Ӧ����ʽ
    add_css_style();

    // ��ʾ��ӭ����
    show_welcome_screen();

    gtk_main();

    if (snake.body) {
        free(snake.body);
    }
    if (obstacles) {
        free(obstacles);
    }
	place_super_food();
    return 0;
}
