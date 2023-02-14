#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

// This code uses playbin, which is an abstraction of all the elements of the pipeline
int main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.example.Holbify");

    Gtk::Window window;
    window.set_default_size(200, 200);

    // Create a grid layout
    Gtk::Grid grid;
    window.add(grid);

     // Create the menu button
    Gtk::MenuButton menuButton;
    menuButton.set_direction(Gtk::ArrowType::ARROW_DOWN);
    grid.attach(menuButton, 0, 0, 1, 1);

    // Create the drop-down menu
    Gtk::Menu menu;
    menuButton.set_popup(menu);

    // Create the Open menu item
    Gtk::MenuItem openMenuItem("Open");
    menu.append(openMenuItem);

    // Create the Quit menu item
    Gtk::MenuItem quitMenuItem("Quit");
    menu.append(quitMenuItem);

    // Connect the Quit menu item to the quit signal
    quitMenuItem.signal_activate().connect([&app] {
        app->quit();
    });

    // Connect the Open menu item to a file chooser dialog
    openMenuItem.signal_activate().connect([&window] {
        Gtk::FileChooserDialog dialog("Please choose a file",
        Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(window);

        // Add buttons to the dialog
        dialog.add_button("Open", Gtk::ResponseType::RESPONSE_OK);
        dialog.add_button("Cancel", Gtk::ResponseType::RESPONSE_CANCEL);

        // Show the dialog
        int result = dialog.run();

        // Handle the response
        switch (result) {
        case Gtk::ResponseType::RESPONSE_OK:
            std::cout << "File selected: " << dialog.get_filename() << std::endl;
            break;
        case Gtk::ResponseType::RESPONSE_CANCEL:
            std::cout << "Cancelled" << std::endl;
            break;
        default:
            break;
        }
    });

    // Create the play/pause button
    Gtk::ToggleButton playPauseButton("Play/Pause");
    grid.attach(playPauseButton, 0, 1, 1, 1);

    // Create the stop button
    Gtk::Button stopButton("Stop");
    grid.attach(stopButton, 1, 1, 1, 1);

    Glib::RefPtr<Gst::Pipeline> pipeline = Gst::Pipeline::create("my-pipeline");

    Glib::RefPtr<Gst::Element> playbin = Gst::ElementFactory::create_element("playbin");
    std::string file_path("/home/rb5014/Holbify/1.mp3");
    playbin->set_property("uri", Glib::filename_to_uri(file_path));

    pipeline->add(playbin);

    playPauseButton.signal_clicked().connect([&] {
        Gst::State current, pending;
        pipeline->get_state(current, pending, 0);
        if (current != Gst::State::STATE_PLAYING) {
            pipeline->set_state(Gst::State::STATE_PLAYING);
        }
        else {
            pipeline->set_state(Gst::State::STATE_PAUSED);
        }
    });

    stopButton.signal_clicked().connect([&] {
        pipeline->set_state(Gst::State::STATE_NULL);
    });

    window.show_all();
    return app->run(window);
}
