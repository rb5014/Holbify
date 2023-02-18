#include "functions.h"
#include <cstdlib>
#include <gtkmm.h>
#include <gstreamermm.h>
#include <gst/gst.h>
#include <gst/tag/tag.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <iostream>
#include <memory>
#include <deque>

bool is_function_updating = false;
bool is_song_playing = false;
std::deque <Glib::ustring> playedSongs;

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

    w.previousButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_previous_button_clicked), playbin, std::ref(w)));

    w.forwardButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_forward_button_clicked), playbin, std::ref(w)));

    w.stopButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_stop_button_clicked), playbin, std::ref(w)));

    w.volumeButton->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_volume_value_changed), playbin));

    w.scaleBar->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_scaleBar_value_changed), playbin, std::ref(w)));

    Glib::RefPtr<Gst::Bus> bus = playbin->get_bus();
    bus->add_signal_watch();
    bus->signal_message().connect(sigc::bind(sigc::ptr_fun(on_message), playbin, std::ref(w)));

    Glib::signal_timeout().connect([&]() {
        Gst::State state, pending_state;
        playbin->get_state(state, pending_state, 0);
        if (state == Gst::State::STATE_PLAYING) {
            return update_scale_bar(playbin, std::ref(w));
        }
        return true;
    }, 100);

}

void on_message(Glib::RefPtr<Gst::Message> message, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    auto msgType = static_cast<GstMessageType>(message->get_message_type());
    //g_print("Received message of type %s\n", gst_message_type_get_name(msgType));
    if (message->get_message_type() == Gst::MESSAGE_TAG) {
        auto tag_message = Glib::RefPtr<Gst::MessageTag>::cast_static(message);
        if (tag_message) {
            Gst::TagList tag_list = tag_message->parse_tag_list();
            gchar *title;
            if(gst_tag_list_get_string(tag_list.gobj(), GST_TAG_TITLE, &title)) {
                w.songInfoLabel->set_label(title);
            }
        }
    } else if (message->get_message_type() == Gst::MESSAGE_STATE_CHANGED) {
        GstState newState;
        gst_message_parse_state_changed(message->gobj(), nullptr, &newState, nullptr);
        if (newState == GST_STATE_PLAYING) {
            is_song_playing = true;
            gint64 len = get_song_length(playbin);
            // Set the range of the scale bar to the length of the song
            update_length_label(playbin, std::ref(w));
            w.scaleBar->set_range(0, len / GST_SECOND);
        }     
    }
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

bool update_scale_bar(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    gint64 pos = get_current_position(playbin);

    //update_song_info_label(playbin, std::ref(w));
    // Update the value of the scale bar
    is_function_updating = true;
    w.scaleBar->set_value(pos / GST_SECOND);
    is_function_updating = false;


    // Format the position and length as minutes and seconds
    std::string pos_str = convert_time(pos);

    // Update the position label
    w.positionLabel->set_label(pos_str);

    return true;
}

/*void update_song_info_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    Gst::TagList* tagList;
    playbin->get_property("tags", tagList);

    if (!tagList)
        return;

    gchar* title;
    if (gst_tag_list_get_string(tagList->gobj(), GST_TAG_TITLE, &title)) {
        w.songInfoLabel->set_text(title);
        g_free(title);
    }

    gchar* artist;
    if (gst_tag_list_get_string(tagList, GST_TAG_ARTIST, &artist)) {
        label.set_text(label.get_text() + " - " + artist);
        g_free(artist);
    }
    

    gst_tag_list_unref(tagList->gobj());
}
*/




void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {

    gint64 len = get_song_length(playbin);
    std::string len_str = convert_time(len);
    w.lengthLabel->set_label(len_str);
}

/*
void start_song(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->set_state(Gst::State::STATE_PLAYING);
}
*/

void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->set_state(Gst::State::STATE_NULL);
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::State::STATE_PLAYING);
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
        Glib::ustring filename = w.fileChooserDialog->get_filename();
        file_opener(filename, playbin, std::ref(w));
        playedSongs.push_back(filename);
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
    builder->get_widget("previousButton", w.previousButton);
    builder->get_widget("forwardButton", w.forwardButton);
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

void on_previous_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    g_print("IN IIIIIIT");
    // If the deque has at least two songs, remove the current song and get the previous song
    if (playedSongs.size() > 1) {
        std::string currentSong = playedSongs.back();
        playedSongs.pop_back();
        playedSongs.push_front(currentSong);
        std::string previousSong = playedSongs.back();

        // Set the state of the playbin to null, update the URI to the previous song, and start playing
        file_opener(previousSong, playbin, std::ref(w));
    }
    else {
        // If the deque only has one song, just replay the current song
        playbin->set_state(Gst::State::STATE_NULL);
        playbin->set_state(Gst::State::STATE_PLAYING);
    }
}

void on_forward_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    if (!playedSongs.empty()) {
        // Get the next song in the deque
        std::string nextSong = playedSongs.front();
        if (playedSongs.size() > 1) {
            playedSongs.pop_front();
        }

        // Stop the current song and play the next one
        file_opener(nextSong, playbin, std::ref(w));
    }
}

void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    Gst::State current, pending;
    playbin->get_state(current, pending, 0);
    if (current == Gst::State::STATE_PLAYING) {
        playbin->set_state(Gst::State::STATE_PAUSED);
    }
    else if (current == Gst::State::STATE_PAUSED) {
        playbin->set_state(Gst::State::STATE_PLAYING);
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
    playbin->set_state(Gst::State::STATE_NULL);
    playbin->set_state(Gst::State::STATE_READY);
    playbin->set_state(Gst::State::STATE_PAUSED);
    // Reset the value of the scale bar to 0
    w.scaleBar->set_value(0);
    is_function_updating = false;

    // Update the two time labels with the new value
    w.positionLabel->set_label("--:--");
    w.lengthLabel->set_label("--:--");
    w.songInfoLabel->set_label("");
}   



void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_volume() = value;
}
