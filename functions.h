#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstdlib>
#include <gtkmm.h>
#include <gstreamermm.h>
#include <gst/gst.h>
#include <gst/tag/tag.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <iostream>


Glib::RefPtr<Gst::PlayBin> create_playbin();
Glib::RefPtr<Gtk::Application> create_application(int argc, char *argv[]);

struct widgets {
    Gtk::Window* window;
    Gtk::MenuItem* openMenuItem;
    Gtk::FileChooserDialog* fileChooserDialog;
    Gtk::MenuItem* quitMenuItem;
    Gtk::Button* playPauseButton;
    Gtk::Button* stopButton;
    Gtk::Button* openFile;
    Gtk::Button* cancelOpen;
    Gtk::VolumeButton* volumeButton;
    Gtk::Scale* scaleBar;
    Gtk::Label* positionLabel;
    Gtk::Label* lengthLabel;
    Gtk::Label* songInfoLabel;
};
widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder);

void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app);
void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void file_chooser(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_message(Glib::RefPtr<Gst::Message> message, widgets &w);
void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_scaleBar_value_changed(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin);
void start_song(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
bool update_scale_bar(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
void update_song_info_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);

#endif
