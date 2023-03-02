#include "functions.h"


// Interpret messages of the bus
void on_message(Glib::RefPtr<Gst::Message> message, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    auto msgType = message->get_message_type();
    //g_print("Received message of type %s\n", gst_message_type_get_name(msgType));
    auto play_icon = Gtk::manage(new Gtk::Image("icons/play.png"));
    auto pause_icon = Gtk::manage(new Gtk::Image("icons/pause.png"));
    if (msgType == Gst::MESSAGE_STATE_CHANGED) {
        GstState newState;
        gst_message_parse_state_changed(message->gobj(), nullptr, &newState, nullptr);
        if (newState == GST_STATE_READY || newState == GST_STATE_PAUSED) {
            if (newState == GST_STATE_READY) {
                reset_visuals(std::ref(w));
            }
            w.playPauseButton->set_image(*play_icon);
            w.playPauseButton->set_tooltip_text("Play");
        } else if (newState == GST_STATE_PLAYING) {
            w.playPauseButton->set_image(*pause_icon);
            w.playPauseButton->set_tooltip_text("Pause");
        } 
    } else if (msgType == Gst::MESSAGE_TAG) {
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
    } else if (msgType == Gst::MESSAGE_EOS) {
        on_next_button_clicked(playbin, w);
    }
}

// Seek to the position in the song when user changes scale value
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

// Update the scale bar every 100 milliseconds
bool on_timeout_update_scale_bar(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
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


// Shows a file chooser dialog to open a file
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
        playedSongs.push_front(filename); // add new song to the front of deque
    }
    if (result == Gtk::RESPONSE_CANCEL) {
        w.fileChooserDialog->hide();
    }
    w.fileChooserDialog->hide();
}

// Go to previous song or go to the end of the playlist
void on_previous_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
   
    if (!currentPlaylist.empty()) {
        if (currentSongIndex == 0) {
            // If the current song is the first song in the vector, wrap around to the last song
            currentSongIndex = currentPlaylist.size() -1;
        } else {
            currentSongIndex--;
        }

        // Set the URI of the playbin to the previous song and start playing
        file_opener(currentPlaylist[currentSongIndex], playbin, w);
    } else if (!playedSongs.empty()) {
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

}

// Go to next song or come back to beginning of the playlist
void on_next_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    if (!currentPlaylist.empty()) {
        if (currentSongIndex == currentPlaylist.size() -1) {
            // If the current song is the last song in the vector, wrap around to the first song
            currentSongIndex = 0;
        } else {
            currentSongIndex++;
        }
        // Set the URI of the playbin to the next song and start playing
        file_opener(currentPlaylist[currentSongIndex], playbin, w);
    } else if (!playedSongs.empty()) {
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
}

// Start or pause the music or open filechooser if no files loaded yet
void on_play_pause_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    Gst::State current, pending;
    playbin->get_state(current, pending, 0);
    if (current == Gst::State::STATE_PLAYING) {
        playbin->set_state(Gst::State::STATE_PAUSED);
    }
    else { 
        if (current == Gst::State::STATE_PAUSED || current == Gst::State::STATE_READY) {
            playbin->set_state(Gst::State::STATE_PLAYING);
        }
        else {
            file_chooser(playbin, std::ref(w));
        }
    }
}

// Stops the currently playing song, resets the scale bar and time label to their initial values
void on_stop_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w)
{
    playbin->set_state(Gst::State::STATE_READY);

}

// Set the volume to the new value 
void on_volume_value_changed(double value, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_volume() = value;
}

// Open a file chooser dialog to create a playlist
void on_create_playlist_button_clicked() {
    // Create a playlist vector
    std::vector<std::string> playlist;
    
    // Create the file chooser dialog
    Gtk::FileChooserDialog dialog("Select Files", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_select_multiple(true);
    // Add the "Create" and "Cancel" buttons to the file chooser dialog
    dialog.add_button("Create", Gtk::RESPONSE_OK);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

    // Add an audio files filter to the dialog
    add_audio_filter(dialog);
    // Create the "Add" button
    Gtk::Button* addButton = Gtk::manage(new Gtk::Button("Add"));

    // Create the list box and add it to a scrolled window
    Gtk::ListBox listBox;
    listBox.set_selection_mode(Gtk::SELECTION_MULTIPLE);
    Gtk::ScrolledWindow scrolledWindow;
    scrolledWindow.add(listBox);
    scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledWindow.set_size_request(300, 200);

    // Create a vertical box to hold the scrolled window and the "Add" button
    Gtk::Box vbox(Gtk::ORIENTATION_VERTICAL, 5);
    vbox.pack_start(scrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    vbox.pack_start(*addButton, Gtk::PACK_SHRINK);

    // Add the vertical box to the file chooser dialog
    dialog.get_content_area()->add(vbox);

    // Create the playlist name label and entry
    Gtk::Label playlistNameLabel("Enter playlist name:");
    Gtk::Entry playlistNameEntry;
    playlistNameEntry.set_hexpand(true);
    playlistNameEntry.set_placeholder_text("My Playlist");

    // Create a horizontal box to hold the playlist name label and entry
    Gtk::Box playlistNameBox(Gtk::ORIENTATION_HORIZONTAL, 5);
    playlistNameBox.pack_start(playlistNameLabel, Gtk::PACK_SHRINK);
    playlistNameBox.pack_start(playlistNameEntry, Gtk::PACK_EXPAND_WIDGET);

    // Add the playlist name box to the content area
    dialog.get_content_area()->add(playlistNameBox);

    // Show the dialog and wait for the user to respond
    dialog.show_all(); // display all widgets in the dialog

    addButton->signal_clicked().connect([&] {on_add_button_clicked(dialog, listBox, playlist);
        // Connect the row-activated signal to the on_list_box_row_activated function
        listBox.signal_row_activated().connect([&](Gtk::ListBoxRow* row){
            on_list_box_row_activated(dialog, listBox, *row, playlist);
        });
    });

    int response = dialog.run();

    if (response == Gtk::RESPONSE_OK) {
        // Get the playlist name from the entry widget
        std::string playlistName = playlistNameEntry.get_text();

        // If the user did not enter a name, use a default name
        if (playlistName.empty()) {
            playlistName = "My Playlist";
        }

        // Save the playlist
        std::string filename = playlistName + ".txt";
        save_playlist(dialog, playlist, filename);
    }
}

// Add files to the listbox in the file chooser dialog for the playlist
void on_add_button_clicked(Gtk::FileChooserDialog& dialog, Gtk::ListBox& listBox, std::vector<std::string>& playlist) {
    auto selectedFiles = dialog.get_filenames();
    for (const auto& file : selectedFiles) {
        if (is_directory(file)) {
            // Add all audio files in the directory
            auto audioFiles = get_audio_files_in_directory(file);
            for (const auto& audioFile : audioFiles) {
                if (std::find(playlist.begin(), playlist.end(), audioFile) == playlist.end()) {
                    playlist.push_back(audioFile);
                    auto label = Gtk::manage(new Gtk::Label(g_path_get_basename(audioFile.c_str())));
                    label->set_data("filepath", new std::string(audioFile));
                    listBox.add(*label);
                }
            }
        } else if (is_audio_file(file)) {
            if (std::find(playlist.begin(), playlist.end(), file) == playlist.end()) {
                playlist.push_back(file);
                auto label = Gtk::manage(new Gtk::Label(g_path_get_basename(file.c_str())));
                label->set_data("filepath", new std::string(file));
                listBox.add(*label);
            }
        }
    }
    dialog.show_all();
}

// Remove files from the listBox when clicked on
void on_list_box_row_activated(Gtk::FileChooserDialog& dialog, Gtk::ListBox& listBox, Gtk::ListBoxRow& row, std::vector<std::string>& playlist) {
    // Get the selected filename from the row
    auto child = row.get_child();
    auto label = dynamic_cast<Gtk::Label*>(child);
    std::string filename = label->get_text();
    auto data = label->get_data("filepath");
    auto filepath = *static_cast<std::string*>(data);

    // Remove the filename from the list box and playlist
    if (row.get_parent()) {
        listBox.remove(row);
    } else {
        label->set_text("");
    }
    playlist.erase(std::remove(playlist.begin(), playlist.end(), filepath), playlist.end());
    dialog.show_all();
}

// Open and starts a playlist
void on_open_playlist_button_clicked(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    // Create the file chooser dialog and set its title
    Gtk::FileChooserDialog dialog(*w.mainWindow, "Open Playlist", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_current_folder("./Playlists");

    // Add the "Open" and "Cancel" buttons to the file chooser dialog
    dialog.add_button("Open", Gtk::RESPONSE_OK);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

    // Show the dialog and wait for the user to respond
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        // Get the selected filename from the dialog
        std::string filename = dialog.get_filename();

        // Load the playlist from the file
        currentPlaylist = load_playlist(filename);

        // Call the fileopener with the first file in the playlist
        if (!currentPlaylist.empty()) {
            file_opener(currentPlaylist[0], playbin, w);
        }
    }
}
