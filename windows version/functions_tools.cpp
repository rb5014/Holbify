#include "functions.h"

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


// Update the length of the song label
void update_length_label(Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {

    gint64 len = get_song_length(playbin);
    std::string len_str = convert_time(len);
    w.lengthLabel->set_label(len_str);
}

// Is the file a directory or not
bool is_directory(const std::string& path) {
    return Glib::file_test(path, Glib::FILE_TEST_IS_DIR);
}

// Is the file an audio file or not
bool is_audio_file(const std::string& filename) {
    static const std::unordered_set<std::string> audio_extensions {
        ".mp3", ".wav", ".flac", ".m4a", ".ogg", ".wma"
    };

    // Check if the file has an audio file extension
    auto pos = filename.find_last_of(".");
    if (pos == std::string::npos) {
        return false;
    }

    std::string extension = filename.substr(pos);
    return audio_extensions.count(extension) > 0;
}

// Get all audio files in a directory
std::vector<std::string> get_audio_files_in_directory(const std::string& path) {
    std::vector<std::string> audioFiles;
    DIR* dir = opendir(path.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG && is_audio_file(entry->d_name)) {
                audioFiles.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
    return audioFiles;
}

// Create a file filter that only allows audio files
void add_audio_filter(Gtk::FileChooserDialog& dialog) {
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Audio Files");
    filter->add_mime_type("audio/*");
    dialog.add_filter(filter);
}

// Starts playing a song, set the playbin 
void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    playbin->set_state(Gst::State::STATE_NULL);
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::State::STATE_PLAYING);
    currentSong = filename;
}

// Load an existing playlist
std::vector<std::string> load_playlist(const std::string& filename) {
    // Open the file for reading
    std::ifstream file(filename);

    // Read each line from the file and add it to the playlist
    std::vector<std::string> playlist;
    std::string line;
    while (std::getline(file, line)) {
        playlist.push_back(line);
    }
    // Close the file and return the playlist
    file.close();
    return playlist;
}

// Save a playlist 
void save_playlist(Gtk::FileChooserDialog& dialog, const std::vector<std::string>& playlist, const std::string& filename) {
    // Check if the Playlists directory exists, and create it if it doesn't
    if (!g_file_test("Playlists", G_FILE_TEST_IS_DIR)) {
        mkdir("Playlists", 0777);
    }

    // Build the full file path
    std::string full_path = "Playlists/" + filename;

    // Check if the file already exists, and prompt the user to replace it if it does
    if (g_file_test(full_path.c_str(), G_FILE_TEST_IS_REGULAR)) {
        Gtk::MessageDialog dialog(dialog, "File already exists. Replace it?", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        int result = dialog.run();
        if (result == Gtk::RESPONSE_NO) {
            return;
        }
    }
    // Open the file for writing
    std::ofstream file(full_path);

    // Write each song path to the file
    for (const auto& songPath : playlist) {
        file << songPath << std::endl;
    }

    // Close the file
    file.close();
}

// Reset visuals, when user hit stop for example
void reset_visuals(widgets& w) {
    // Reset the labels and the values
    w.positionLabel->set_label("--:--");
    w.lengthLabel->set_label("--:--");
    w.mainWindow->set_title("Music Player Holbify");
    // Set boolean to true to prevent on_scaleBar_value_changed to be called
    is_function_updating = true;
    // Reset the value of the scale bar to 0
    w.scaleBar->set_value(0);
    is_function_updating = false;
}