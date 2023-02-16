#include "functions.h"
#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>


int main(int argc, char *argv[])
{
    // Create the app
    Glib::RefPtr<Gtk::Application> app = create_application(argc, argv);

    // Create the playbin
    Glib::RefPtr<Gst::PlayBin> playbin = create_playbin();

    // Load the Glade XML file
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("GUI.glade");

    // Load the widgets from the Glade XML file
    widgets w = load_widgets(builder);

    connect_signals(w, playbin, app);

    return app->run(*w.window);
}