#include <gtk/gtk.h>
#include <string.h>

#define PAGE_INFO_NUM (6)

static void assistant_cancel (GtkAssistant *, gpointer);
static void assistant_close (GtkAssistant *, gpointer);

typedef struct
{
    GtkWidget *widget;
    gint index;
    const gchar *title;
    GtkAssistantPageType type;
    gboolean complete;
} PageInfo;


int
main (int argc, char *argv[])
{
    GtkWidget *assistant;
    guint i;
    PageInfo page[PAGE_INFO_NUM] = {
        {NULL, -1, "欢迎使用设置向导", GTK_ASSISTANT_PAGE_INTRO, TRUE},
        {NULL, -1, "输入法引擎选择", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, "个性皮肤选择", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, "候选词栏设置", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, "快捷键设置", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
        {NULL, -1, "向导设置完成", GTK_ASSISTANT_PAGE_CONFIRM, TRUE},
    };

    gtk_init (&argc, &argv);

    /* Create a new assistant widget with no pages. */
    assistant = gtk_assistant_new ();
    gtk_widget_set_size_request (assistant, 600, 450);
    gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);
    gtk_window_set_title (GTK_WINDOW (assistant), "Fcitx配置向导");
    g_signal_connect (G_OBJECT (assistant), "destroy",
                         G_CALLBACK (gtk_main_quit), NULL);

    page[0].widget = gtk_label_new (NULL);
    gchar* s = g_strdup_printf("<b>欢迎使用个性化设置向导！</b>");
    gtk_label_set_markup(GTK_LABEL(page[0].widget), s);

    GtkWidget* imwidget = gtk_image_new_from_file((gchar*)"./002.png");
    page[1].widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(page[1].widget), imwidget, TRUE, TRUE, 0);

  
    page[2].widget = gtk_label_new ("step 2 test");
    page[3].widget = gtk_label_new ("step 3 test");
    page[4].widget = gtk_label_new ("step 4 test");
    page[5].widget = gtk_label_new ("step 5 test");

    /* Add five pages to the GtkAssistant dialog. */
    for (i = 0; i < PAGE_INFO_NUM; i++) {
        page[i].index = gtk_assistant_append_page (GTK_ASSISTANT (assistant),
                                                      page[i].widget);
        gtk_assistant_set_page_title (GTK_ASSISTANT (assistant),
                                         page[i].widget, page[i].title);
        gtk_assistant_set_page_type (GTK_ASSISTANT (assistant),
                                        page[i].widget, page[i].type);

        /* Set the introduction and conclusion pages as complete so they can be
         * incremented or closed. */
        gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant),
                                            page[i].widget, page[i].complete);
    }

    g_signal_connect (G_OBJECT (assistant), "cancel",
                         G_CALLBACK (assistant_cancel), NULL);
    g_signal_connect (G_OBJECT (assistant), "close",
                         G_CALLBACK (assistant_close), NULL);
    gtk_widget_show_all (assistant);
    gtk_main ();
    return 0;
}

/* If the dialog is cancelled, delete it from memory and then clean up after
 * the Assistant structure. */
static void
assistant_cancel (GtkAssistant * assistant, gpointer data)
{
    gtk_widget_destroy (GTK_WIDGET (assistant));
}

/* This function is where you would apply the changes and destroy the assistant. */
static void
assistant_close (GtkAssistant * assistant, gpointer data)
{
    g_print ("You would apply your changes now!\n");
    gtk_widget_destroy (GTK_WIDGET (assistant));
}

