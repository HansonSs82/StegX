/*
 * This file is part of the StegX project.
 * Copyright (C) 2018  StegX Team
 * 
 * StegX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file signals.c
 * @brief Configuration des signaux
 * @details Configure les signaux de l'interface utilisateur. Ne gère que les
 * signaux et les interfactions, ne concerne pas l'affichage statique.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gtk/gtk.h>
#include "stegx.h"

#include "common/config.h"
#include "struct.h"
#include "misc.h"

/* Variables.
 * ============================================================================= */

/**
 * @brief Liste des noms des algorithmes.
 * @internal Les noms doivent être dans l'ordre de l'énumération de la
 * bibliothèque.
 * @author Pierre Ayoub et Damien Delaunay
 * */
static const char *algos_lst[] = {
    "Least Significant Bit", "End-Of-File", "Metadonnée",
    "End-Of-Chunk", "Junk chunk"
};

/** Permet de faire la correspondance entre l'indice de l'algorithme dans la liste
 * proposé à l'utilisateur et l'indice de l'algorithme dans l'énumération. */
static int algos_ind[STEGX_NB_ALGO] = { -1 };

/**
 * Variable globale au module signal qui contient le numéro de l'état de
 * l'interface graphique pour le module dissimulation.
 * @details L'état correspond à si on doit effectuer l'analyse (0) ou effectuer
 * la dissmulation (1).
 * @author Pierre Ayoub et Damien Delaunay
 */
static int insert_state = 0;

/* Fonctions (déclarations).
 * ============================================================================= */

/** 
 * Lancement de l'étape courante de l'onglet dissimulation.
 * @details Lance l'analyse des fichiers entrés par l'utilisateur ou l'insertion
 * des données du fichier à cacher dans le fichier hôte.
 * @sideeffect Met à jour l'interface pendant le traitement et lance le thread
 * chargé du traitement.
 * @param widget Bouton qui a émis le signal.
 * @param ui Structure de l'interface utilisateur.
 * @author Pierre Ayoub et Damien Delaunay
 */
static void insert_start(GtkWidget * widget, struct ui *ui);

/** 
 * Exécution de l'étape courante de l'onglet dissimulation.
 * @details Exécute l'analyse des fichiers entrés par l'utilisateur ou
 * l'insertion des données du fichier caché dans le fichier hôte.
 * @sideeffect Met à jour la variable \r{stegx_errno} avec le code de retour du
 * traitement.
 * @param data Structure de l'interface utilisateur.
 * @return gboolean Code indiquant de supprimer le thread.
 * @author Pierre Ayoub et Damien Delaunay
 */
static gboolean insert_do(gpointer data);

/**
 * Termine l'étape courante de l'onglet dissimulation.
 * @details Récupère le code de retour de la fonction de la bibliothèque. Si
 * tout est ok, si on effectué l'analyse on passe à l'étape suivante ; si on à
 * effectué l'insertion on revient à l'état inital. Sinon, on revient à la même
 * étape.
 * @sideeffect Met à jours les widgets et affiche les messages de dialogue en
 * conséquence.
 * @param ui Structure de l'interface utilisateur.
 * @author Pierre Ayoub et Damien Delaunay
 */
static void insert_end(struct ui *ui);

/**
 * Réinitialisation de la dissimulation.
 * @sideeffect Réinitialisation de l'affichage des widgets dans l'onglet insertion
 * et revient dans l'état initial d'analyse.
 * @param widget Bouton qui a émis le signal.
 * @param ui Structure de l'interface utilisateur.
 * @author Pierre Ayoub et Damien Delaunay
 */
static void insert_reset(GtkWidget * widget, struct ui *ui);

/**
 * Lancement de l'extraction.
 * @details Lance l'extraction du fichier caché.
 * @sideffect Met à jour l'interface pendant le traitement et lance le thread
 * chargé de de l'extraction.
 * @param widget Bouton qui a émis le signal.
 * @param ui Structure de l'interface utilisateur.
 * @author Pierre Ayoub et Damien Delaunay
 */
static void extrac_start(GtkWidget * widget, struct ui *ui);

/** 
 * Exécution de l'extraction.
 * @details Exécute l'extraction des données du fichier caché dans le fichier
 * hôte.
 * @sideeffect Met à jour la variable \r{stegx_errno} avec le code de retour de
 * l'extraction.
 * @param data Structure de l'interface utilisateur.
 * @return gboolean Code indiquant de supprimer le thread.
 * @author Pierre Ayoub et Damien Delaunay
 */
static gboolean extrac_do(gpointer data);

/**
 * Termine l'extraction.
 * @details Récupère le code de retour de la fonction de la bibliothèque.
 * @sideeffect Met à jour les widgets et affiche les messages de dialogue en
 * conséquence.
 * @param ui Structure de l'interface utilisateur.
 * @author Pierre Ayoub et Damien Delaunay
 */
static void extrac_end(struct ui *ui);

/* Fonctions (implémentation).
 * ============================================================================= */

static void insert_start(GtkWidget * widget, struct ui *ui)
{
    /* Si les paramètres nécéssaires ont bien étés remplis par l'utilisateur. */
    if (gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_orig_fc))
        && gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_to_hide_fc))
        && gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_out_dir_fc))
        && strcmp(gtk_entry_get_text(GTK_ENTRY(ui->insert.file_out_name_ent)), "")) {
        /* Création du dialogue à afficher pour patienter. */
        gchar *msg = insert_state == 0 ? ui->insert.dial_anal_proc : ui->insert.dial_dissi_proc;
        ui->insert.dial = ui_msg_dial_new(ui->window, msg, UI_DIAL_INFO_WAIT);
        /* Lancement du thread de travaille. */
        gdk_threads_add_idle(insert_do, ui);
        /* On met à jour le texte du bouton et on affiche le dialogue. */
        gtk_button_set_label(GTK_BUTTON(widget), ui->insert.but_txt_dissi_proc);
        gtk_dialog_run(GTK_DIALOG(ui->insert.dial));
    } else {
        /* Création du dialogue d'avertissement puis affichage. */
        gchar *msg = insert_state == 0 ? ui->insert.dial_anal_cond : ui->insert.dial_dissi_cond;
        ui->insert.dial = ui_msg_dial_new(ui->window, msg, UI_DIAL_WARN);
        gtk_dialog_run(GTK_DIALOG(ui->insert.dial));
        gtk_widget_destroy(ui->insert.dial);
    }
}

static gboolean insert_do(gpointer data)
{
    struct ui *ui = (struct ui *)data;  /* Interface. */
    static stegx_choices_s steg_choices;        /* Structure publique. */
    static stegx_info_insert_s insert_info;     /* Structure d'insertion publique. */
    static info_s *steg_info;   /* Structure privée. */

    /* Si on doit faire l'analyse. */
    if (!insert_state) {
        /* Remplissage des champs de la structure publique. */
        steg_choices.insert_info = &insert_info;
        // Fichier hôte.
        steg_choices.host_path =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_orig_fc));
        // Mot de passe.
        steg_choices.passwd = strlen(gtk_entry_get_text(GTK_ENTRY(ui->insert.passwd_ent))) ?
            strdup(gtk_entry_get_text(GTK_ENTRY(ui->insert.passwd_ent))) : NULL;
        // Mode.
        steg_choices.mode = STEGX_MODE_INSERT;
        // Fichier à cacher.
        insert_info.hidden_path =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_to_hide_fc));
        // Fichier résultat. 
        steg_choices.res_path =
            calloc(strlen(gtk_entry_get_text(GTK_ENTRY(ui->insert.file_out_name_ent))) + 2 +
                   strlen(gtk_file_chooser_get_filename
                          (GTK_FILE_CHOOSER(ui->insert.file_out_dir_fc))), sizeof(char));
        strcat(steg_choices.res_path,
               gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->insert.file_out_dir_fc)));
        strcat(steg_choices.res_path, "/");
        strcat(steg_choices.res_path, gtk_entry_get_text(GTK_ENTRY(ui->insert.file_out_name_ent)));

        /* Initialisation de la bibliothèque avec les choix puis vérifie la
         * compatibilité. Enfin, analyse quels algorithmes peuvent êtres
         * utilisés. On s'arrête s'il y a une erreur dans une de ces fonctions. */
        if ((steg_info = stegx_init(&steg_choices)) && !stegx_check_compatibility(steg_info))
            stegx_suggest_algo(steg_info);
    }
    /* Si c'est la dissimulation. */
    else {
        /* Sélection de l'algorithme et lancement de la dissimulation. */
        if (!stegx_choose_algo
            (steg_info, algos_ind[gtk_combo_box_get_active(GTK_COMBO_BOX(ui->insert.algos_cb))]))
            stegx_insert(steg_info);
    }

    /* Libération de la mémoire et fermeture des flux si on a terminé la
     * dissimulation sans erreur ou s'il y a eu une erreur à l'analyse
     * (=> prochaine étape = début de l'analyse). */
    if ((!insert_state && stegx_errno) || (insert_state && !stegx_errno)) {
        stegx_clear(steg_info);
        free(steg_choices.passwd), free(steg_choices.res_path);
    }

    /* Suppression du dialogue en cours. */
    gtk_dialog_response(GTK_DIALOG(ui->insert.dial), 0);
    gtk_widget_destroy(ui->insert.dial);
    /* Fin du traitement puis suppression du thread. */
    insert_end(ui);
    return G_SOURCE_REMOVE;
}

static void insert_end(struct ui *ui)
{
    /* Si le traitement à réussi. */
    if (!stegx_errno) {
        gchar *msg = (insert_state == 0) ? ui->insert.dial_anal_end : ui->insert.dial_dissi_end;
        ui->insert.dial = ui_msg_dial_new(ui->window, msg, UI_DIAL_INFO_OK);
        /* Si c'est l'analyse. */
        if (!insert_state) {
            /* Mise à jour du bouton pour la dissimulation. */
            gtk_button_set_label(GTK_BUTTON(ui->insert.but), ui->insert.but_txt_dissi);
            /* Retire les widgets de sélection des paramètre de stéganographie. */
            gtk_widget_hide(ui->insert.file_orig_fc), gtk_widget_hide(ui->insert.file_orig_lbl);
            gtk_widget_hide(ui->insert.file_to_hide_fc),
                gtk_widget_hide(ui->insert.file_to_hide_lbl);
            gtk_widget_hide(ui->insert.file_out_dir_fc),
                gtk_widget_hide(ui->insert.file_out_dir_lbl);
            gtk_widget_hide(ui->insert.file_out_name_ent),
                gtk_widget_hide(ui->insert.file_out_name_lbl);
            gtk_widget_hide(ui->insert.passwd_ent), gtk_widget_hide(ui->insert.passwd_lbl);
            /* Ajoute le choix de sélection de l'algorithme. */
            for (algo_e i = 0, ind = 0; i < STEGX_NB_ALGO; i++) {
                if (stegx_propos_algos[i]) {
                    algos_ind[ind++] = i;       /* indice de l'algorithme dans la liste <=> indice de l'algorithme dans l'énumération. */
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->insert.algos_cb),
                                                   algos_lst[i]);
                }
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->insert.algos_cb), 0);
            gtk_widget_show(ui->insert.algos_lbl);
            gtk_widget_show(ui->insert.algos_cb);
            insert_state = 1;
        }
        /* Si c'est l'insertion. */
        else
            insert_reset(NULL, ui);
    }
    /* Si le traitement à échoué. */
    else {
        gchar *msg = (insert_state == 0) ? ui->insert.dial_anal_err : ui->insert.dial_dissi_err;
        ui->insert.dial = ui_msg_dial_new(ui->window, msg, UI_DIAL_ERR);
        /* Mise à jour du bouton pour relancer le traitement. */
        gtk_button_set_label(GTK_BUTTON(ui->insert.but), insert_state == 0 ?
                             ui->insert.but_txt_anal : ui->insert.but_txt_dissi);
        /* Affichage de l'erreur précise sur stderr. */
        err_print(stegx_errno);
        stegx_errno = ERR_NONE;
    }
    /* Affichage du dialogue de fin. */
    gtk_dialog_run(GTK_DIALOG(ui->insert.dial));
    gtk_widget_destroy(ui->insert.dial);
}

static void insert_reset(GtkWidget * widget, struct ui *ui)
{
    (void)widget;               /* Unused. */
    /* Ré-affiche les widgets de choix de stéganographie. */
    gtk_widget_show(ui->insert.file_orig_fc), gtk_widget_show(ui->insert.file_orig_lbl);
    gtk_widget_show(ui->insert.file_to_hide_fc), gtk_widget_show(ui->insert.file_to_hide_lbl);
    gtk_widget_show(ui->insert.file_out_dir_fc), gtk_widget_show(ui->insert.file_out_dir_lbl);
    gtk_widget_show(ui->insert.file_out_name_ent), gtk_widget_show(ui->insert.file_out_name_lbl);
    gtk_widget_show(ui->insert.passwd_ent), gtk_widget_show(ui->insert.passwd_lbl);
    /* Cache le widget permettant de choisir l'algorithme et le réinitialise. */
    gtk_widget_hide(ui->insert.algos_lbl);
    gtk_widget_hide(ui->insert.algos_cb);
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ui->insert.algos_cb));
    /* Re-configure le bon label du bouton. */
    gtk_button_set_label(GTK_BUTTON(ui->insert.but), ui->insert.but_txt_anal);
    /* Ré-initialisation de l'état. */
    insert_state = 0;
    stegx_errno = ERR_NONE;
}

static void extrac_start(GtkWidget * widget, struct ui *ui)
{
    /* Si les paramètres nécéssaires ont bien étés remplis par l'utilisateur. */
    if (gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->extrac.file_out_dir_fc))
        && gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->extrac.file_orig_fc))) {
        /* Création du dialogue d'attente puis lancement du thread de travaille. */
        ui->extrac.dial = ui_msg_dial_new(ui->window, ui->extrac.dial_proc, UI_DIAL_INFO_WAIT);
        gdk_threads_add_idle(extrac_do, ui);
        /* On met à jour le texte du bouton et on affiche le dialogue précédemment créé. */
        gtk_button_set_label(GTK_BUTTON(widget), ui->extrac.but_txt_proc);
        gtk_dialog_run(GTK_DIALOG(ui->extrac.dial));
    } else {
        /* Création du dialogue d'avertissement puis affichage. */
        ui->extrac.dial = ui_msg_dial_new(ui->window, ui->extrac.dial_cond, UI_DIAL_WARN);
        gtk_dialog_run(GTK_DIALOG(ui->extrac.dial));
        gtk_widget_destroy(ui->extrac.dial);
    }
}

static gboolean extrac_do(gpointer data)
{
    struct ui *ui = (struct ui *)data;  /* Interface. */
    static stegx_choices_s steg_choices;        /* Structure publique. */
    static info_s *steg_info;   /* Structure privée. */

    /* Remplissage des champs de la structure publique. */
    steg_choices.insert_info = NULL;
    // Fichier hôte.
    steg_choices.host_path =
        gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->extrac.file_orig_fc));
    // Mot de passe.
    steg_choices.passwd = strlen(gtk_entry_get_text(GTK_ENTRY(ui->extrac.passwd_ent))) ?
        strdup(gtk_entry_get_text(GTK_ENTRY(ui->extrac.passwd_ent))) : NULL;
    // Mode.
    steg_choices.mode = STEGX_MODE_EXTRACT;
    // Fichier résultat. 
    steg_choices.res_path =
        calloc(strlen(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->extrac.file_out_dir_fc))) +
               2, sizeof(char));
    strcat(steg_choices.res_path,
           gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->extrac.file_out_dir_fc)));
    strcat(steg_choices.res_path, "/");

    /* Initialisation de la bibliothèque avec les choix puis vérifie la
     * compatibilité. Ensuite, analyse le fichier hôte et l'algorithme
     * utilisé lors de la dissimulation. Finalement, on lance l'extraction
     * du fichier caché. On s'arrête s'il y a une erreur dans une de ces
     * fonctions. */
    if ((steg_info = stegx_init(&steg_choices)) && !stegx_check_compatibility(steg_info)
        && !stegx_detect_algo(steg_info))
        stegx_extract(steg_info, steg_choices.res_path);

    /* Libération de la mémoire et fermeture des flux. */
    stegx_clear(steg_info);
    free(steg_choices.passwd), free(steg_choices.res_path);

    /* Suppression du dialogue en cours. */
    gtk_dialog_response(GTK_DIALOG(ui->extrac.dial), 0);
    gtk_widget_destroy(ui->extrac.dial);
    /* Fin du traitement puis suppression du thread. */
    extrac_end(ui);
    return G_SOURCE_REMOVE;
}

static void extrac_end(struct ui *ui)
{
    /* Si l'extraction à réussie. */
    if (!stegx_errno)
        ui->extrac.dial = ui_msg_dial_new(ui->window, ui->extrac.dial_end, UI_DIAL_INFO_OK);
    /* Si l'extraction à échouée. */
    else {
        ui->extrac.dial = ui_msg_dial_new(ui->window, ui->extrac.dial_err, UI_DIAL_ERR);
        /* Affichage de l'erreur précise sur stderr. */
        err_print(stegx_errno);
        stegx_errno = ERR_NONE;
    }
    /* Mise à jour du bouton pour relancer l'extraction. */
    gtk_button_set_label(GTK_BUTTON(ui->extrac.but), ui->extrac.but_txt);
    /* Affichage du dialogue de fin. */
    gtk_dialog_run(GTK_DIALOG(ui->extrac.dial));
    gtk_widget_destroy(ui->extrac.dial);
}

/**
 * Affiche le dialogue "à propos".
 * @details Construit et affiche le dialogue d'informations "à propos".
 * @param widget Bouton qui a émis le signal.
 * @param ui Structure de l'interface utilisateur.
 */
static void about(GtkWidget * widget, struct ui *ui)
{
    /* Paramètres. */
    static const gchar *authors[] = {
        "AYOUB Pierre", "BASKEVITCH Claire", "BESSAC Tristan",
        "CAUMES Clément", "DELAUNAY Damien", "DOUDOUH Yassin",
        NULL
    };
    static const gchar *copyright = "© 2018 StegX Team";
    static const gchar *website = "https://github.com/Heisenberk/StegX";
    GdkPixbuf *logo = gdk_pixbuf_new_from_file(program_logo(), NULL);
    /* Initilisation et affichage. */
    gtk_show_about_dialog(GTK_WINDOW(ui->window),
                          "authors", authors, "logo", logo,
                          "license", program_license, "website", website,
                          "comments", program_desc, "copyright", copyright,
                          "version", program_ver, "program_name", program_name, NULL);
    gtk_menu_item_deselect(GTK_MENU_ITEM(widget));
}

void ui_signal_init(struct ui *ui)
{
    /* Signaux de l'onglet insertion. */
    g_signal_connect(ui->insert.but, "clicked", G_CALLBACK(insert_start), ui);
    g_signal_connect(ui->insert.but_reset, "clicked", G_CALLBACK(insert_reset), ui);
    /* Signaux de l'onglet extraction. */
    g_signal_connect(ui->extrac.but, "clicked", G_CALLBACK(extrac_start), ui);
    /* Signaux du menu utilisateur. */
    g_signal_connect(ui->menu.about, "select", G_CALLBACK(about), ui);
}
