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
#include <deque>
#include <libgen.h>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <unordered_set>

struct widgets {
    Gtk::Window* mainWindow;
    Gtk::MenuItem* openMenuItem;
    Gtk::MenuItem* quitMenuItem;
    Gtk::MenuItem* createPlaylistButton;
    Gtk::MenuItem* openPlaylistButton;
    Gtk::FileChooserDialog* fileChooserDialog;
    Gtk::FileChooserDialog* createPlaylistDialog;
    Gtk::ListBox* fileListBox;
    Gtk::Button* playPauseButton;
    Gtk::Button* previousButton;
    Gtk::Button* nextButton;
    Gtk::Button* stopButton;
    Gtk::Button* openFile;
    Gtk::Button* cancelOpen;
    Gtk::VolumeButton* volumeButton;
    Gtk::Scale* scaleBar;
    Gtk::Label* positionLabel;
    Gtk::Label* lengthLabel;
};
// functions_init.cpp
Glib::RefPtr<Gst::PlayBin> create_playbin();
Glib::RefPtr<Gtk::Application> create_application(int argc, char *argv[]);
widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder);
void load_icons(widgets& w);
void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app);

// functions_events.cpp
void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void file_chooser(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void add_audio_filter(Gtk::FileChooserDialog& dialog);
void on_message(Glib::RefPtr<Gst::Message> message, Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_previous_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
void on_next_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
void on_scaleBar_value_changed(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin);
//void start_song(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
bool update_scale_bar(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w);
void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
void update_song_info_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);
void reset_visuals(widgets& w);
void on_add_button_clicked(Gtk::FileChooserDialog& dialog, Gtk::ListBox& listBox, std::vector<std::string>& playlist);
void on_list_box_row_activated(Gtk::FileChooserDialog& dialog, Gtk::ListBox& listBox, Gtk::ListBoxRow& row, std::vector<std::string>& playlist);
void save_playlist(Gtk::FileChooserDialog& dialog, const std::vector<std::string>& playlist, const std::string& filename);
void on_create_playlist_button_clicked();
std::vector<std::string> load_playlist(const std::string& filename);
void on_open_playlist_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w);

// functions_tools.cpp
bool is_directory(const std::string& path);
bool is_audio_file(const std::string& filename);
std::vector<std::string> get_audio_files_in_directory(const std::string& path);
gint64 get_current_position(Glib::RefPtr<Gst::PlayBin> playbin);
gint64 get_song_length(Glib::RefPtr<Gst::PlayBin> playbin);
std::string convert_time(gint64 time_ns);
#endif
