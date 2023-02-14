#include <gtk/gtk.h>
#include <gstreamer-1.0/gst/gst.h>
#include <cstdlib>
#include <iostream>
#include <glib.h>


gboolean update_scale_bar(GtkRange *scaleBar, GstElement *playbin)
{
    gint64 pos, len;
    GstFormat format = GST_FORMAT_TIME;

    // Get the current position and length of the song in time format
    if (gst_element_query_position(playbin, format, &pos) && gst_element_query_duration(playbin, format, &len))
    {
        // Calculate the percentage of the song that has been played
        double value = static_cast<double>(pos) / static_cast<double>(len);

        // Set the range of the scale bar to the length of the song
        gtk_range_set_range(scaleBar, 0, len / GST_SECOND);

        // Format the position and length as minutes and seconds
        gint pos_min = (pos / GST_SECOND) / 60;
        gint pos_sec = (pos / GST_SECOND) % 60;
        gint len_min = (len / GST_SECOND) / 60;
        gint len_sec = (len / GST_SECOND) % 60;

        // Create a string that displays the current time of the song and the total length of the song
        gchar* time_str = g_strdup_printf("%d:%02d / %d:%02d", pos_min, pos_sec, len_min, len_sec);

        // Update the value and tooltip text of the scale bar
        gtk_range_set_value(scaleBar, pos / GST_SECOND);
        gtk_widget_set_tooltip_text(GTK_WIDGET(scaleBar), time_str);
        g_free(time_str);
    }

    return G_SOURCE_CONTINUE;
}


extern "C" G_MODULE_EXPORT void open_file_button_clicked(GtkButton* button, gpointer user_data)
{
    auto playbin = Glib::RefPtr<Gst::PlayBin>::cast_static(*static_cast<Glib::RefPtr<Gst::PlayBin>*>(user_data));
    GtkWidget* parent_window = gtk_widget_get_parent_window(GTK_WIDGET(button));
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(parent_window), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar* uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
        playbin->set_state(Gst::STATE_NULL);
        playbin->set_property("uri", uri);
        playbin->set_state(Gst::STATE_PLAYING);
        g_free(uri);
    }
    gtk_widget_destroy(dialog);
}

extern "C" G_MODULE_EXPORT void play_pause_button_clicked(GtkButton* button, gpointer user_data)
{
    auto playbin = Glib::RefPtr<Gst::PlayBin>::cast_static(*static_cast<Glib::RefPtr<Gst::PlayBin>*>(user_data));
    Gst::State current_state, pending_state;
    playbin->get_state(current_state, pending_state, 0);
    if (current_state == Gst::STATE_PLAYING) {
        playbin->set_state(Gst::STATE_PAUSED);
    } else {
        playbin->set_state(Gst::STATE_PLAYING);
    }
}

extern "C" G_MODULE_EXPORT void stop_button_clicked(GtkButton* button, gpointer user_data)
{
    auto playbin = Glib::RefPtr<Gst::PlayBin>::cast_static(*static_cast<Glib::RefPtr<Gst::PlayBin>*>(user_data));
    playbin->set_state(Gst::STATE_NULL);
}

extern "C" G_MODULE_EXPORT void volume_scale_value_changed(GtkScale* scale, gpointer user_data)
{
    auto playbin = Glib::RefPtr<Gst::PlayBin>::cast_static(*static_cast<Glib::RefPtr<Gst::PlayBin>*>(user_data));
    playbin->set_property("volume", static_cast<double>(gtk_range_get_value(GTK_RANGE(scale))));
}


int main(int argc, char* argv[])
{
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the main window
    GtkWidget* mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(mainWindow), "Holbify Music Player");

    // Create the playbin
    GstElement* playbin = gst_element_factory_make("playbin", "playbin");

    // Load the UI from the Glade file
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "GUI.glade", NULL);

    // Get the widgets from the UI
    GtkWidget* openFileButton = GTK_WIDGET(gtk_builder_get_object(builder, "openFileButton"));
    GtkWidget* playPauseButton = GTK_WIDGET(gtk_builder_get_object(builder, "playPauseButton"));
    GtkWidget* stopButton = GTK_WIDGET(gtk_builder_get_object(builder, "stopButton"));
    GtkWidget* volumeScale = GTK_WIDGET(gtk_builder_get_object(builder, "volumeScale"));
    GtkWidget* songScale = GTK_WIDGET(gtk_builder_get_object(builder, "songScale"));

    // Set up the callbacks for the buttons
    g_signal_connect(G_OBJECT(openFileButton), "clicked", G_CALLBACK(open_file_button_clicked), playbin);
    g_signal_connect(G_OBJECT(playPauseButton), "clicked", G_CALLBACK(play_pause_button_clicked), playbin);
    g_signal_connect(G_OBJECT(stopButton), "clicked", G_CALLBACK(stop_button_clicked), playbin);
    g_signal_connect(G_OBJECT(volumeScale), "value-changed", G_CALLBACK(volume_scale_value_changed), playbin);

    // Connect the update_scale_bar function to the timeout signal of the GTK main loop
    g_timeout_add(100, (GSourceFunc)update_scale_bar, songScale);

    // Show the main window and run the main loop
    gtk_widget_show_all(mainWindow);
    gtk_main();

    return 0;
}
