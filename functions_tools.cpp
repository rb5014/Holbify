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

bool is_directory(const std::string& path) {
    return Glib::file_test(path, Glib::FILE_TEST_IS_DIR);
}

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