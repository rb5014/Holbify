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
std::deque <Glib::ustring> playedSongs;
Glib::ustring currentSong;
long unsigned int currentSongIndex;

void on_message(Glib::RefPtr<Gst::Message> message, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    auto msgType = message->get_message_type();
    //g_print("Received message of type %s\n", gst_message_type_get_name(msgType));
    if (msgType == Gst::MESSAGE_STATE_CHANGED) {
        GstState newState;
        gst_message_parse_state_changed(message->gobj(), nullptr, &newState, nullptr);
        if (newState == GST_STATE_READY) {
            // Update the two time labels with the new value
            w.positionLabel->set_label("--:--");
            w.lengthLabel->set_label("--:--");
            w.mainWindow->set_title("Music Player Holbify");
        }    
    }
    else if (msgType == Gst::MESSAGE_TAG) {
        auto tag_message = Glib::RefPtr<Gst::MessageTag>::cast_static(message);
        if (tag_message) {
            Gst::TagList tag_list = tag_message->parse_tag_list();
            gchar *title;
            if (gst_tag_list_get_string(tag_list.gobj(), GST_TAG_TITLE, &title)) {
                w.mainWindow->set_title(Glib::ustring(title) + " - Music Player Holbify");
            } else {
                w.mainWindow->set_title(Gio::File::create_for_path(currentSong)->get_basename() + " - Music Player Holbify");
            }
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
    gint64 len = get_song_length(playbin);
    update_length_label(playbin, std::ref(w));
    // Set the range of the scale bar to the length of the song
    w.scaleBar->set_range(0, len / GST_SECOND);
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

void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {

    gint64 len = get_song_length(playbin);
    std::string len_str = convert_time(len);
    w.lengthLabel->set_label(len_str);
}


void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->set_state(Gst::State::STATE_NULL);
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::State::STATE_PLAYING);
    currentSong = filename;
}

void file_chooser(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w) {

    // Create a file filter that only allows audio files
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Audio Files");
    filter->add_mime_type("audio/*");
    w.fileChooserDialog->add_filter(filter);

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
        playedSongs.push_front(filename); // add new song to the front of deque
    }
    if (result == Gtk::RESPONSE_CANCEL) {
        w.fileChooserDialog->hide();
    }
    w.fileChooserDialog->hide();
}


void on_previous_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    // If there are no songs in the deque, return
    if (playedSongs.empty()) {
        return;
    }

    // Stop the current song
    playbin->set_state(Gst::State::STATE_NULL);

    // Get the previous song in the deque
    if (currentSongIndex == 0) {
        // If the current song is the first song in the deque, wrap around to the last song
        currentSongIndex = playedSongs.size() - 1;
    } else {
        currentSongIndex--;
    }

    // Set the URI of the playbin to the previous song and start playing
    file_opener(playedSongs[currentSongIndex], playbin, w);
}


void on_forward_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    // If there are no songs in the deque, return
    if (playedSongs.empty()) {
        return;
    }

    // Stop the current song
    playbin->set_state(Gst::State::STATE_NULL);

    // Get the next song in the deque
    if (currentSongIndex == playedSongs.size() - 1) {
        // If the current song is the last song in the deque, wrap around to the first song
        currentSongIndex = 0;
    } else {
        currentSongIndex++;
    }

    // Set the URI of the playbin to the next song and start playing
    file_opener(playedSongs[currentSongIndex], playbin, w);
}


void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    auto play_icon = Gtk::manage(new Gtk::Image("icons/play.png"));
    auto pause_icon = Gtk::manage(new Gtk::Image("icons/pause.png"));
    Gst::State current, pending;
    playbin->get_state(current, pending, 0);
    if (current == Gst::State::STATE_PLAYING) {
        playbin->set_state(Gst::State::STATE_PAUSED);
        w.playPauseButton->set_image(*play_icon);
        w.playPauseButton->set_tooltip_text("Play");
    }
    else { 
        if (current == Gst::State::STATE_PAUSED || current == Gst::State::STATE_READY) {
            playbin->set_state(Gst::State::STATE_PLAYING);
        }
        else {
            file_chooser(playbin, std::ref(w));
        }
        w.playPauseButton->set_image(*pause_icon);
        w.playPauseButton->set_tooltip_text("Pause");
    }
}

// Stops the currently playing song, resets the scale bar and time label to their initial values
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    auto play_icon = Gtk::manage(new Gtk::Image("icons/play.png"));
    w.playPauseButton->set_image(*play_icon);
    w.playPauseButton->set_tooltip_text("Play");
    // Set the state of the playbin to ready
    is_function_updating = true;
    playbin->set_state(Gst::State::STATE_READY);
    // Reset the value of the scale bar to 0
    w.scaleBar->set_value(0);
    is_function_updating = false;
}   

void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_volume() = value;
}

