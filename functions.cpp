#include "functions.h"
#include <cstdlib>
#include <gtkmm.h>
#include <gstreamermm.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <iostream>

bool is_function_updating = false;

Glib::RefPtr<Gst::PlayBin> create_playbin() {
    Gst::init();
    return Gst::PlayBin::create("playbin");
}

Glib::RefPtr<Gtk::Application> create_application(int argc, char *argv[]) {
    Glib::init();
    return Gtk::Application::create(argc, argv, "com.example.Holbify");
}

void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app) {
    w.quitMenuItem->signal_activate().connect([&] {app->quit(); });

    w.openMenuItem->signal_activate().connect(sigc::bind(sigc::ptr_fun(&file_chooser), playbin, std::ref(w)));

    w.playPauseButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_play_pause_button_clicked), playbin, std::ref(w)));

    w.stopButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_stop_button_clicked), playbin, std::ref(w)));

    w.volumeButton->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_volume_value_changed), playbin));

    Glib::signal_timeout().connect([&]() {
        Gst::State state, pending_state;
        playbin->get_state(state, pending_state, 0);
        if (state == Gst::State::STATE_PLAYING) {
            return update_scale_bar(playbin, std::ref(w));
        }
        return true;
    }, 100);

    w.scaleBar->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_scaleBar_value_changed), playbin, std::ref(w)));

}

void on_scaleBar_value_changed(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    if (is_function_updating)
        return;
    gint64 len = 0;
    playbin->query_duration(Gst::FORMAT_TIME, len);
    gint64 pos = w.scaleBar->get_value() * GST_SECOND;

    // Seek to the new position in the song
    playbin->seek(Gst::FORMAT_TIME, Gst::SeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), pos);

    // Update the tooltip text of the scale bar
    int pos_min = (pos / GST_SECOND) / 60;
    int pos_sec = (pos / GST_SECOND) % 60;
    w.scaleBar->set_tooltip_text(Glib::ustring::compose("%1:%2", pos_min, pos_sec));
}

// Get the current position of the song in time format
gint64 get_current_position(Glib::RefPtr<Gst::PlayBin> playbin)
{
    gint64 pos;
    Gst::Format format = Gst::FORMAT_TIME;
    playbin->query_position(format, pos);
    return pos;
}

// Get the length of the song in time format
gint64 get_song_length(Glib::RefPtr<Gst::PlayBin> playbin)
{
    gint64 len;
    Gst::Format format = Gst::FORMAT_TIME;
    playbin->query_duration(format, len);
    return len;
}

// Convert time in nanoseconds to minutes and seconds
std::string convert_time(gint64 time_ns)
{
    int time_sec = time_ns / GST_SECOND;
    int min = time_sec / 60;
    int sec = time_sec % 60;
    std::string time_str = std::to_string(min) + ":" + (sec < 10 ? "0" : "") + std::to_string(sec);
    return time_str;
}

bool update_scale_bar(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    gint64 pos = get_current_position(playbin);
    gint64 len = get_song_length(playbin);

    update_length_label(playbin, std::ref(w));

    // Update the value of the scale bar
    is_function_updating = true;
    w.scaleBar->set_value(pos / GST_SECOND);
    is_function_updating = false;

    // Set the range of the scale bar to the length of the song
    w.scaleBar->set_range(0, len / GST_SECOND);

    // Format the position and length as minutes and seconds
    std::string pos_str = convert_time(pos);

    // Update the position label
    w.positionLabel->set_label(pos_str);

    return true;
}

void update_song_info_label(Gtk::Label& songInfoLabel, Glib::RefPtr<Gst::PlayBin> playbin)
{
    Gst::TagList tags = playbin->get_tags();
    if (tags) {
        Glib::ustring artist, title;
        tags->get_string(Gst::Tag::Artist, artist);
        tags->get_string(Gst::Tag::Title, title);
        Glib::ustring songInfo = artist + " - " + title;
        songInfoLabel.set_text(songInfo);
    }
}


void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {

    gint64 len = get_song_length(playbin);
    std::string len_str = convert_time(len);
    w.lengthLabel->set_label(len_str);
}

void start_song(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->set_state(Gst::State::STATE_PAUSED);
    playbin->set_state(Gst::State::STATE_PLAYING);
    update_length_label(playbin, std::ref(w));
}

void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->property_uri() = Glib::filename_to_uri(filename);
    start_song(playbin, std::ref(w));
}

void file_chooser(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w) {

    w.openFile->signal_clicked().connect([&]{
        w.fileChooserDialog->response(Gtk::RESPONSE_OK);
    });

    w.cancelOpen->signal_clicked().connect([&]{
        w.fileChooserDialog->response(Gtk::RESPONSE_CANCEL);
    });
    int result = w.fileChooserDialog->run();

    // Handle the result of the file chooser dialog
    if (result == Gtk::RESPONSE_OK) {
        playbin->set_state(Gst::State::STATE_NULL);
        Glib::ustring filename = w.fileChooserDialog->get_filename();
        file_opener(filename, playbin, std::ref(w));
    }
    if (result == Gtk::RESPONSE_CANCEL) {
        w.fileChooserDialog->hide();
    }
    w.fileChooserDialog->hide();
}

widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder)
{
    widgets w;
    builder->get_widget("mainWindow", w.window);
    builder->get_widget("openMenuItem", w.openMenuItem);
    builder->get_widget("fileChooserDialog", w.fileChooserDialog);
    builder->get_widget("quitMenuItem", w.quitMenuItem);
    builder->get_widget("playPauseButton", w.playPauseButton);
    builder->get_widget("stopButton", w.stopButton);
    builder->get_widget("openFile", w.openFile);
    builder->get_widget("cancelOpen", w.cancelOpen);
    builder->get_widget("volumeButton", w.volumeButton);
    builder->get_widget("scaleBar", w.scaleBar);
    builder->get_widget("positionLabel", w.positionLabel);
    builder->get_widget("lengthLabel", w.lengthLabel);
    builder->get_widget("songInfoLabel", w.songInfoLabel);
    return w;
}

void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    Gst::State current, pending;
    playbin->get_state(current, pending, 0);
    if (current == Gst::State::STATE_PLAYING) {
        playbin->set_state(Gst::State::STATE_PAUSED);
    }
    else if (current == Gst::State::STATE_PAUSED) {
        start_song(playbin, std::ref(w));
    }
    else {
        file_chooser(playbin, std::ref(w));
    }
}

// Stops the currently playing song, resets the scale bar and time label to their initial values
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    // Set the state of the playbin to ready
    is_function_updating = true;
    playbin->set_state(Gst::State::STATE_READY);
    playbin->set_state(Gst::State::STATE_PAUSED);
    // Reset the value of the scale bar to 0
    w.scaleBar->set_value(0);
    is_function_updating = false;

    // Update the two time labels with the new value
    w.positionLabel->set_label("--:--");
    w.lengthLabel->set_label("--:--");
}



void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_volume() = value;
}
