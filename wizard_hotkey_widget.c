/***************************************************************************
 *   Copyright (C) 2010~2012 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <fcitx-utils/utils.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>

#include "common.h"
#include "wizard_hotkey_widget.h"
#include "gdm-languages.h"
#include "config_widget.h"
#include "sub_config_widget.h"
#include "configdesc.h"
#include "dummy_config.h"

#include "keygrab.h"

enum {
    PROP_0,

    PROP_CONFIG_DESC,
    PROP_PREFIX,
    PROP_NAME,
    PROP_SUBCONFIG
};


G_DEFINE_TYPE(FcitxWizardHotkeyWidget, fcitx_wizard_hotkey_widget, GTK_TYPE_BOX)

static void fcitx_wizard_hotkey_widget_dispose(GObject* object);

static void
fcitx_wizard_hotkey_widget_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec);


static GObject *
fcitx_wizard_hotkey_widget_constructor(GType gtype,
    guint n_properties, GObjectConstructParam *properties);

static void
fcitx_wizard_hotkey_widget_class_init(FcitxWizardHotkeyWidgetClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->set_property = fcitx_wizard_hotkey_widget_set_property;
    gobject_class->dispose = fcitx_wizard_hotkey_widget_dispose;
    gobject_class->constructor = fcitx_wizard_hotkey_widget_constructor;

    g_object_class_install_property(gobject_class, PROP_CONFIG_DESC,
        g_param_spec_pointer("cfdesc", "Configuration Description",
        "Configuration Description for this widget", 
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class, PROP_PREFIX,
        g_param_spec_string("prefix", "Prefix of path",
        "Prefix of configuration path", NULL,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class, PROP_NAME,
        g_param_spec_string("name", "File name",
        "File name of configuration file", NULL,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(gobject_class,
        PROP_SUBCONFIG, g_param_spec_string("subconfig",
        "subconfig", "subconfig", NULL,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

}


static GObject *
fcitx_wizard_hotkey_widget_constructor(GType gtype,
    guint n_properties, GObjectConstructParam *properties)
{
    GObject *obj;
    obj = G_OBJECT_CLASS(fcitx_wizard_hotkey_widget_parent_class)->constructor(gtype, 
        n_properties, properties);

    return obj;
}

static void
fcitx_wizard_hotkey_widget_init(FcitxWizardHotkeyWidget* self)
{
    //do nothing
}

static void
fcitx_wizard_hotkey_widget_load_conf(FcitxWizardHotkeyWidget *self)
{
    FILE *fp;
    FcitxConfigValueType value;

    if (self->cfdesc == NULL) {
        FcitxLog(WARNING, _("Parameter self->cfdesc is NULL.\n"));
        return;
    }

    bindtextdomain(self->cfdesc->domain, LOCALEDIR);
    bind_textdomain_codeset(self->cfdesc->domain, "UTF-8");

    self->config = dummy_config_new(self->cfdesc);
    
    if ((fp = FcitxXDGGetFileWithPrefix(self->prefix, self->name, "r", NULL))
        == NULL) 
    {
        FcitxLog(WARNING, _("Open file(%s/%s) error.\n"), self->prefix, self->name);
        return;
    }

    dummy_config_load(self->config, fp);
    dummy_config_sync(self->config);

    fclose(fp);

    value = FcitxConfigGetBindValue(&self->config->config, "Hotkey", "TriggerKey");
    keygrab_button_set_key(KEYGRAB_BUTTON(self->trigger_key_button[0]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].state);
    keygrab_button_set_key(KEYGRAB_BUTTON(self->trigger_key_button[1]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].state);

    value = FcitxConfigGetBindValue(&self->config->config, "Hotkey", "IMSwitchKey");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->im_switch_key_button),
        *value.boolvalue); 

    value = FcitxConfigGetBindValue(&self->config->config, "Hotkey", "IMSwitchHotkey");
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->im_switch_hotkey_combo), *value.enumerate);

    value = FcitxConfigGetBindValue(&self->config->config, "Hotkey", "PrevPageKey");
    keygrab_button_set_key(KEYGRAB_BUTTON(self->prev_page_button[0]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].state);
    keygrab_button_set_key(KEYGRAB_BUTTON(self->prev_page_button[1]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].state);

    value = FcitxConfigGetBindValue(&self->config->config, "Hotkey", "NextPageKey");
    keygrab_button_set_key(KEYGRAB_BUTTON(self->next_page_button[0]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[0].state);
    keygrab_button_set_key(KEYGRAB_BUTTON(self->next_page_button[1]), 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].sym, 
        ((FcitxHotkeys *)value.hotkey)->hotkey[1].state);

}


void 
fcitx_wizard_hotkey_widget_data_changed(FcitxWizardHotkeyWidget *self)
{
    FILE *fp;
    GError  *error;
    gchar *argv[3];
    
    if ((fp = FcitxXDGGetFileUserWithPrefix(self->prefix, self->name, "w", NULL))
        == NULL) 
    {
        FcitxLog(WARNING, _("Open file(%s/%s) error.\n"), self->prefix, self->name);
        return;
    }

    FcitxConfigSaveConfigFileFp(fp, &self->config->config, self->cfdesc);
    fclose(fp);

    argv[0] = EXEC_PREFIX "/bin/fcitx-remote";
    argv[1] = "-r";
    argv[2] = 0;
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
   
}

void 
trigger_key_button_changed(GtkWidget* button, 
    gpointer user_data)
{
    guint key;
    GdkModifierType mods;
    FcitxWizardHotkeyWidget *self = user_data;
    
    keygrab_button_get_key(KEYGRAB_BUTTON(self->trigger_key_button[0]), &key, &mods);
    self->conf_data.trigger_key.hotkey[0].sym = key;
    self->conf_data.trigger_key.hotkey[0].state = mods;
    self->conf_data.trigger_key.hotkey[0].desc = FcitxHotkeyGetKeyString(key, mods);

    keygrab_button_get_key(KEYGRAB_BUTTON(self->trigger_key_button[1]), &key, &mods);
    self->conf_data.trigger_key.hotkey[1].sym = key;
    self->conf_data.trigger_key.hotkey[1].state = mods;
    self->conf_data.trigger_key.hotkey[1].desc = FcitxHotkeyGetKeyString(key, mods);
    

    FcitxConfigBindValue(self->config->config.configFile, "Hotkey", "TriggerKey", 
        &self->conf_data.trigger_key, NULL, NULL);

    fcitx_wizard_hotkey_widget_data_changed(self);
}    

void 
im_switch_key_button_toggled(GtkWidget* button, 
    gpointer user_data)
{
    FcitxWizardHotkeyWidget *self = user_data;

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
        self->conf_data.im_switch_key = true;
    else
        self->conf_data.im_switch_key = false;

    FcitxConfigBindValue(self->config->config.configFile, "Hotkey", "IMSwitchKey", 
        &self->conf_data.im_switch_key, NULL, NULL);

    fcitx_wizard_hotkey_widget_data_changed(self);
}

void 
im_switch_hotkey_combo_changed(GtkWidget* combo, 
    gpointer user_data)
{
    FcitxWizardHotkeyWidget *self = user_data;

    self->conf_data.im_switch_hotkey = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

    FcitxConfigBindValue(self->config->config.configFile, "Hotkey", "IMSwitchHotkey", 
        &self->conf_data.im_switch_hotkey, NULL, NULL);

    fcitx_wizard_hotkey_widget_data_changed(self);
}

void 
prev_page_button_changed(GtkWidget* button, 
    gpointer user_data)
{
    guint key;
    GdkModifierType mods;
    FcitxWizardHotkeyWidget *self = user_data;
    
    keygrab_button_get_key(KEYGRAB_BUTTON(self->prev_page_button[0]), &key, &mods);
    self->conf_data.prev_page.hotkey[0].sym = key;
    self->conf_data.prev_page.hotkey[0].state = mods;
    self->conf_data.prev_page.hotkey[0].desc = FcitxHotkeyGetKeyString(key, mods);
    
    keygrab_button_get_key(KEYGRAB_BUTTON(self->prev_page_button[1]), &key, &mods);
    self->conf_data.prev_page.hotkey[1].sym = key;
    self->conf_data.prev_page.hotkey[1].state = mods;
    self->conf_data.prev_page.hotkey[1].desc = FcitxHotkeyGetKeyString(key, mods);


    FcitxConfigBindValue(self->config->config.configFile, "Hotkey", "PrevPageKey", 
        &self->conf_data.prev_page, NULL, NULL);

    fcitx_wizard_hotkey_widget_data_changed(self);
}    

void 
next_page_button_changed(GtkWidget* button, 
    gpointer user_data)
{
    guint key;
    GdkModifierType mods;
    FcitxWizardHotkeyWidget *self = user_data;
    
    keygrab_button_get_key(KEYGRAB_BUTTON(self->next_page_button[0]), &key, &mods);
    self->conf_data.next_page.hotkey[0].sym = key;
    self->conf_data.next_page.hotkey[0].state = mods;
    self->conf_data.next_page.hotkey[0].desc = FcitxHotkeyGetKeyString(key, mods);

    keygrab_button_get_key(KEYGRAB_BUTTON(self->next_page_button[1]), &key, &mods);
    self->conf_data.next_page.hotkey[1].sym = key;
    self->conf_data.next_page.hotkey[1].state = mods;
    self->conf_data.next_page.hotkey[1].desc = FcitxHotkeyGetKeyString(key, mods);

    FcitxConfigBindValue(self->config->config.configFile, "Hotkey", "NextPageKey", 
        &self->conf_data.next_page, NULL, NULL);

    fcitx_wizard_hotkey_widget_data_changed(self);
}    

static void
fcitx_wizard_hotkey_widget_setup_ui(FcitxWizardHotkeyWidget *self)
{
    int row = 0;
    GtkWidget *cvbox = GTK_WIDGET(self);

    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_margin_left(grid, 0);
    gtk_widget_set_margin_top(grid, 6);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);

    GtkWidget* label = gtk_label_new(("Trigger Input Method"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->trigger_key_button[0] = keygrab_button_new();
    self->trigger_key_button[1] = keygrab_button_new();
    GtkWidget *inputWidget = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(inputWidget), 
        self->trigger_key_button[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(inputWidget), 
        self->trigger_key_button[1], 1, 0, 2, 1);
    g_object_set(G_OBJECT(self->trigger_key_button[0]), 
        "hexpand", TRUE, NULL);
    g_object_set(G_OBJECT(self->trigger_key_button[1]), 
        "hexpand", TRUE, NULL);
    g_object_set(inputWidget, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputWidget, 1, row, 1, 1);

    row ++;
    label = gtk_label_new(("Enable Hotkey to scroll Between Input Method"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->im_switch_key_button = gtk_check_button_new();
    g_object_set(self->im_switch_key_button, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->im_switch_key_button, 1, row, 1, 1);

    row ++;
    label = gtk_label_new(("Scroll between Input Method"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->im_switch_hotkey_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(self->im_switch_hotkey_combo), 
        "CTRL_SHIFT");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(self->im_switch_hotkey_combo), 
        "ALT_SHIFT");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(self->im_switch_hotkey_combo), 
        "CTRL_SUPER");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(self->im_switch_hotkey_combo), 
        "ALT_SUPER");
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->im_switch_hotkey_combo), 0);
    g_object_set(self->im_switch_hotkey_combo, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->im_switch_hotkey_combo, 1, row, 1, 1);

    row ++;
    label = gtk_label_new(("Prev Page"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->prev_page_button[0] = keygrab_button_new();
    self->prev_page_button[1] = keygrab_button_new();
    inputWidget = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(inputWidget), self->prev_page_button[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(inputWidget), self->prev_page_button[1], 1, 0, 2, 1);
    g_object_set(G_OBJECT(self->prev_page_button[0]), "hexpand", TRUE, NULL);
    g_object_set(G_OBJECT(self->prev_page_button[1]), "hexpand", TRUE, NULL);
    g_object_set(inputWidget, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputWidget, 1, row, 1, 1);

    row ++;
    label = gtk_label_new(("Next Page"));
    g_object_set(label, "xalign", 0.0f, NULL);
    self->next_page_button[0] = keygrab_button_new();
    self->next_page_button[1] = keygrab_button_new();
    inputWidget = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(inputWidget), self->next_page_button[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(inputWidget), self->next_page_button[1], 1, 0, 2, 1);
    g_object_set(G_OBJECT(self->next_page_button[0]), "hexpand", TRUE, NULL);
    g_object_set(G_OBJECT(self->next_page_button[1]), "hexpand", TRUE, NULL);
    g_object_set(inputWidget, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputWidget, 1, row, 1, 1);

    gtk_box_pack_start(GTK_BOX(cvbox), grid, TRUE, TRUE, 0);

    fcitx_wizard_hotkey_widget_load_conf(self);

    g_signal_connect(G_OBJECT(self->trigger_key_button[0]), "changed",
                     G_CALLBACK(trigger_key_button_changed), self);
    g_signal_connect(G_OBJECT(self->trigger_key_button[1]), "changed",
                     G_CALLBACK(trigger_key_button_changed), self);
    g_signal_connect(G_OBJECT(self->im_switch_key_button), "toggled",
                     G_CALLBACK(im_switch_key_button_toggled), self);
    g_signal_connect(G_OBJECT(self->im_switch_hotkey_combo), "changed",
                     G_CALLBACK(im_switch_hotkey_combo_changed), self);
    g_signal_connect(G_OBJECT(self->prev_page_button[0]), "changed",
                     G_CALLBACK(prev_page_button_changed), self);
    g_signal_connect(G_OBJECT(self->prev_page_button[1]), "changed",
                     G_CALLBACK(prev_page_button_changed), self);
    g_signal_connect(G_OBJECT(self->next_page_button[0]), "changed",
                     G_CALLBACK(next_page_button_changed), self);
    g_signal_connect(G_OBJECT(self->next_page_button[1]), "changed",
                     G_CALLBACK(next_page_button_changed), self);

}


GtkWidget*
fcitx_wizard_hotkey_widget_new(FcitxConfigFileDesc* cfdesc, const gchar* prefix, 
    const gchar* name, const gchar* subconfig)
{
    FcitxWizardHotkeyWidget* widget =
        g_object_new(FCITX_TYPE_WIZARD_HOTKEY_WIDGET,
            "cfdesc", cfdesc, "prefix", 
            prefix, "name", name, "subconfig", subconfig, NULL);

    fcitx_wizard_hotkey_widget_setup_ui(widget);

    return GTK_WIDGET(widget);
}

void fcitx_wizard_hotkey_widget_dispose(GObject* object)
{
    FcitxWizardHotkeyWidget* self = FCITX_WIZARD_HOTKEY_WIDGET(object);
    if (self->name) {
        g_free(self->name);
        self->name = NULL;
    }

    if (self->prefix) {
        g_free(self->prefix);
        self->prefix = NULL;
    }

    if (self->parser) {
        sub_config_parser_free(self->parser);
        self->parser = NULL;
    }
 
    if (self->config) {
        dummy_config_free(self->config);
        self->config = NULL;
    }
 
    G_OBJECT_CLASS (fcitx_wizard_hotkey_widget_parent_class)->dispose (object);
}


static void
fcitx_wizard_hotkey_widget_set_property(GObject *gobject,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
    FcitxWizardHotkeyWidget* config_widget = FCITX_WIZARD_HOTKEY_WIDGET(gobject);
    switch (prop_id) {
    case PROP_CONFIG_DESC:
        config_widget->cfdesc = g_value_get_pointer(value);
        break;
    case PROP_PREFIX:
        if (config_widget->prefix)
            g_free(config_widget->prefix);
        config_widget->prefix = g_strdup(g_value_get_string(value));
        break;
    case PROP_NAME:
        if (config_widget->name)
            g_free(config_widget->name);
        config_widget->name = g_strdup(g_value_get_string(value));
        break;
    case PROP_SUBCONFIG:
        if (config_widget->parser)
            sub_config_parser_free(config_widget->parser);
        config_widget->parser = sub_config_parser_new(g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}


