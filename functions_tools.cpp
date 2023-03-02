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

void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w) {
    flagTag = false;
    gtk_image_set_from_file(w.imageBox->gobj(), "logo.png");
    currentCoverPath = "";
    playbin->set_state(Gst::State::STATE_NULL);
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::State::STATE_PLAYING);
    Gst::State state, pending_state;
    // Wait for the state change to playing 
    do {
        playbin->get_state(state, pending_state, 0);
    } while (state != Gst::State::STATE_PLAYING);

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

// Callback function for writing the downloaded data into a buffer
static size_t my_curl_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    std::string *response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}


std::string url_encode(const std::string& str) {
    CURL *curl = curl_easy_init();
    char *output = curl_easy_escape(curl, str.c_str(), str.length());
    std::string encoded(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    return encoded;
}

// Builds the Last.fm API request URL with the provided parameters
std::string buildLastfmRequestUrl(const std::string& track, const std::string& artist, const std::string& album, const std::string& api_key) {
    std::string url = "https://ws.audioscrobbler.com/2.0/?method=track.getInfo&api_key=cc48ca82a97f232c4208e1a4110ee528";
    if (!track.empty()) {
        url += "&track=" + url_encode(track);
    }
    if (!artist.empty()) {
        url += "&artist=" + url_encode(artist);
    }
    url += "&format=json";
    return url;
}

// Makes a HTTP GET request to the specified URL and returns the response body
std::string makeHttpGetRequest(const std::string& url) {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_curl_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            return response;
        }
    }

    return "";
}

// Parses the Last.fm API response and returns the cover image URL and the album name
std::pair<std::string, std::string> parseLastfmCoverUrl(const std::string& response) {
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(response, root);
    if (parsingSuccessful) {
        Json::Value track = root["track"];
        if (!track.empty()) {
            Json::Value album = track["album"];
            if (!album.empty()) {
                std::string albumName = album["title"].asString();
                Json::Value image = album["image"].isArray() ? album["image"][2]["#text"] : album["image"]["#text"];
                if (!image.empty()) {
                    std::string imageUrl = image.asString();
                    return std::make_pair(imageUrl, albumName);
                }
            }
        }
    }
    return std::make_pair("", "");
}

void downloadImage(const char* url, const char* filename) {
    if (!g_file_test("CoverArts", G_FILE_TEST_IS_DIR)) {
        mkdir("CoverArts", 0777);
    }

    GFile* file = g_file_new_for_path(filename);

    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
        SoupSession* session = soup_session_new();
        SoupMessage* message = soup_message_new("GET", url);

        soup_session_send_message(session, message);
        g_object_unref(session);

        GError* error = NULL;
        GFileOutputStream* stream = g_file_create(file, G_FILE_CREATE_REPLACE_DESTINATION, NULL, &error);

        if (error != NULL) {
            g_print("Error creating file: %s\n", error->message);
            g_clear_error(&error);
            return;
        }

        gsize bytes_written;
        g_output_stream_write_all(G_OUTPUT_STREAM(stream), message->response_body->data, message->response_body->length, &bytes_written, NULL, &error);

        if (error != NULL) {
            g_print("Error writing to file: %s\n", error->message);
            g_clear_error(&error);
            g_object_unref(stream);
            return;
        }

        g_object_unref(stream);
    }

    return;
}

std::string getLastfmCover(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w, const std::string& track, const std::string& artist, const std::string& album, const std::string& api_key) {

    // Build the request URL
    std::string url = buildLastfmRequestUrl(track, artist, album, api_key);
    // Send the HTTP request to Last.fm and parse the response
    std::string response = makeHttpGetRequest(url);

    // Extract the cover image URL from the response
    std::string coverUrl, albumName;
    std::tie(coverUrl, albumName) = parseLastfmCoverUrl(response);
    if ((coverUrl == "") || (albumName == "")) {
        playbin->set_state(Gst::State::STATE_PLAYING);
        return "";
    }
    // Create the filepath
    std::replace(albumName.begin(), albumName.end(), ' ', '_');
    std::string fullpath = "CoverArts/" + albumName + ".jpg";
    // Download the cover image and create a GdkPixbuf object from the data
    downloadImage(coverUrl.c_str(), fullpath.c_str());
    // Return the filepath
    currentCoverPath = fullpath;
    gtk_image_set_from_file(w.imageBox->gobj(), currentCoverPath.c_str());
    playbin->set_state(Gst::State::STATE_PLAYING);
    return fullpath;
}

/*void getLastfmCoverInBackground(Glib::RefPtr<Gst::PlayBin> playbin, widgets &w, const std::string& track, const std::string& artist, const std::string& album, const std::string& api_key) {
  // create a new thread for the download function
  std::thread getCoverThread(getLastfmCover, playbin, std::ref(w), track, artist, album, api_key);
    playbin->set_state(Gst::State::STATE_PAUSED);

  // detach the thread so that it runs in the background
  getCoverThread.detach();
}*/