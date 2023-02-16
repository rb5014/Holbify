#include "functions.h"
#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

bool is_scale_bar_updating = false;

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

    w.openMenuItem->signal_activate().connect(sigc::bind(sigc::ptr_fun(&file_chooser), std::ref(*w.fileChooserDialog), playbin, std::ref(*w.openFile), std::ref(*w.cancelOpen)));

    w.playPauseButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_play_pause_button_clicked), playbin, std::ref(w)));

    w.stopButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_stop_button_clicked), playbin, std::ref(*w.scaleBar), std::ref(*w.timeLabel), std::ref(*w.songInfoLabel)));

    w.volumeButton->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_volume_value_changed), playbin));

    Glib::signal_timeout().connect(sigc::bind(sigc::ptr_fun(&update_scale_bar), std::ref(*w.scaleBar), playbin, std::ref(*w.timeLabel), std::ref(*w.songInfoLabel)), 100);

    w.scaleBar->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_scaleBar_value_changed), w.scaleBar, playbin));

}

void on_scaleBar_value_changed(Gtk::Scale* scaleBar, Glib::RefPtr<Gst::PlayBin> playbin)
{
    if (is_scale_bar_updating)
        return;
    gint64 len = 0;
    playbin->query_duration(Gst::FORMAT_TIME, len);
    gint64 pos = scaleBar->get_value() * GST_SECOND;

    // Seek to the new position in the song
    playbin->seek(Gst::FORMAT_TIME, Gst::SeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), pos);

    // Update the tooltip text of the scale bar
    int pos_min = (pos / GST_SECOND) / 60;
    int pos_sec = (pos / GST_SECOND) % 60;
    scaleBar->set_tooltip_text(Glib::ustring::compose("%1:%2", pos_min, pos_sec));
}

bool update_scale_bar(Gtk::Scale& scaleBar, Glib::RefPtr<Gst::PlayBin> playbin, Gtk::Label& timeLabel, Gtk::Label& songInfoLabel)
{
    gint64 pos, len;
    Gst::Format format = Gst::FORMAT_TIME;

    // Get the current position and length of the song in time format
    if (playbin->query_position(format, pos) && playbin->query_duration(format, len))
    {
        // Set the range of the scale bar to the length of the song
        scaleBar.set_range(0, len / GST_SECOND);

        // Format the position and length as minutes and seconds
        int pos_min = (pos / GST_SECOND) / 60;
        int pos_sec = (pos / GST_SECOND) % 60;
        int len_min = (len / GST_SECOND) / 60;
        int len_sec = (len / GST_SECOND) % 60;

        // Update the value and tooltip text of the scale bar
        is_scale_bar_updating = true;
        scaleBar.set_value(pos / GST_SECOND);
        is_scale_bar_updating = false;
        timeLabel.set_label(Glib::ustring::compose("%1:%2/%3:%4", pos_min, pos_sec, len_min, len_sec));
    }

    return true;
}
/*
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
*/

void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::STATE_PLAYING);
}

void file_chooser(Gtk::FileChooserDialog& dialog,
                  Glib::RefPtr<Gst::PlayBin> playbin,
                  Gtk::Button& openFile,
                  Gtk::Button& cancelOpen) {

    openFile.signal_clicked().connect([&]{
        dialog.response(Gtk::RESPONSE_OK);
    });

    cancelOpen.signal_clicked().connect([&]{
        dialog.response(Gtk::RESPONSE_CANCEL);
    });
    int result = dialog.run();

    // Handle the result of the file chooser dialog
    if (result == Gtk::RESPONSE_OK) {
        Glib::ustring filename = dialog.get_filename();
        playbin->set_state(Gst::STATE_NULL);
        file_opener(filename, playbin);
    }
    if (result == Gtk::RESPONSE_CANCEL) {
        dialog.hide();
    }
    dialog.hide();
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
    builder->get_widget("timeLabel", w.timeLabel);
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
    else if (current == Gst::State::STATE_PAUSED || current == Gst::State::STATE_READY) {
        playbin->set_state(Gst::State::STATE_PLAYING);
    }
    else {
        file_chooser(*w.fileChooserDialog, playbin, *w.openFile, *w.cancelOpen);
    }
}

void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, Gtk::Scale& scaleBar, Gtk::Label& timeLabel, Gtk::Label& songInfoLabel)
{
    playbin->set_state(Gst::State::STATE_READY);
    scaleBar.set_value(0);
    update_scale_bar(scaleBar, playbin, timeLabel, songInfoLabel);
}

void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_volume() = value;
}
