#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>


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
    Gtk::Label* timeLabel;
    Gtk::Label* songInfoLabel;
};
widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder);

void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app);
void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin);
void file_chooser(Gtk::FileChooserDialog& dialog, Glib::RefPtr<Gst::PlayBin> playbin, Gtk::Button& openFile, Gtk::Button& cancelOpen);
void on_scaleBar_value_changed(Gtk::Scale* scaleBar, Glib::RefPtr<Gst::PlayBin> playbin);
bool update_scale_bar(Gtk::Scale& scaleBar, Glib::RefPtr<Gst::PlayBin> playbin, Gtk::Label& timeLabel, Gtk::Label& songInfoLabel);
void update_song_info_label(Gtk::Label& songInfoLabel, Glib::RefPtr<Gst::PlayBin> playbin);
void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, Gtk::Scale& scaleBar, Gtk::Label& timeLabel, Gtk::Label& songInfoLabel);
void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin);

#endif
