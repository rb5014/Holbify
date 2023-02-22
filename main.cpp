#include "functions.h"

bool is_function_updating = false;
std::deque <Glib::ustring> playedSongs;
std::vector <std::string> currentPlaylist;
Glib::ustring currentSong;
long unsigned int currentSongIndex = 0;

int main(int argc, char *argv[])
{
    // Create the app
    Glib::RefPtr<Gtk::Application> app = create_application(argc, argv);

    // Create the playbin
    Glib::RefPtr<Gst::PlayBin> playbin = create_playbin();

    // Load the Glade XML file
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("GUI.glade");

    // Load the widgets and their icons from the Glade XML file
    widgets w = load_widgets(builder);
    load_icons(w);
    
    // Connect the signals from the widgets to the dedicated functions
    connect_signals(w, playbin, app);
    w.mainWindow->show_all();
    return app->run(*w.mainWindow);
}