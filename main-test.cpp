#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

void add_file_to_pipeline(const Glib::ustring& filename, Glib::RefPtr<Gst::Pipeline> pipeline) {
    Glib::RefPtr<Gst::Element> playbin = Gst::ElementFactory::create_element("playbin");
    Glib::RefPtr<Gst::Element> element_in_pipeline = pipeline->get_element_by_name("playbin");
    std::string file_path(filename);
    playbin->set_property("uri", Glib::filename_to_uri(file_path));
    if (!element_in_pipeline)
    {
        pipeline->add(playbin);
    }
}

void file_opener(Glib::ustring filename, Glib::RefPtr<Gst::Pipeline> pipeline) {
    add_file_to_pipeline(filename, pipeline);
    pipeline->set_state(Gst::State::STATE_PLAYING);
}

void file_chooser(Gtk::FileChooserDialog& dialog,
                  Glib::RefPtr<Gst::Pipeline> pipeline,
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
        file_opener(filename, pipeline);
    }
    if (result == Gtk::RESPONSE_CANCEL) {
        dialog.hide();
    }
    dialog.hide();
}

int main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    // Create the app
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.example.Holbify");

    // Create the pipeline
    Glib::RefPtr<Gst::Pipeline> pipeline = Gst::Pipeline::create("my-pipeline");

    // Load the Glade XML file
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("GUI.glade");

    // Get the widgets from the Glade XML file
    Gtk::Window *window;
    builder->get_widget("mainWindow", window);

    Gtk::MenuItem *openMenuItem;
    builder->get_widget("openMenuItem", openMenuItem);

    Gtk::FileChooserDialog *fileChooserDialog;
    builder->get_widget("fileChooserDialog", fileChooserDialog);

    Gtk::MenuItem *quitMenuItem;
    builder->get_widget("quitMenuItem", quitMenuItem);
    
    Gtk::Button *playPauseButton;
    builder->get_widget("playPauseButton", playPauseButton);

    Gtk::Button *stopButton;
    builder->get_widget("stopButton", stopButton);

    Gtk::Button *openFile;
    builder->get_widget("openFile", openFile);

    Gtk::Button *cancelOpen;
    builder->get_widget("cancelOpen", cancelOpen);


    /* Connect the Menu items */

    // Connect the Quit menu item to the quit signal
    quitMenuItem->signal_activate().connect([&app] {
        app->quit();
    });

    // Connect the Open menu item to the FileChooserDialog opening by using the function on_open_menu_activated
    openMenuItem->signal_activate().connect([&] {file_chooser(*fileChooserDialog, pipeline, *openFile, *cancelOpen);});

    playPauseButton->signal_clicked().connect([&] {
        Gst::State current, pending;
        pipeline->get_state(current, pending, 0);
        if (current == Gst::State::STATE_PLAYING) {
            pipeline->set_state(Gst::State::STATE_PAUSED);
        }
        else if (current == Gst::State::STATE_PAUSED || current == Gst::State::STATE_READY) {
            pipeline->set_state(Gst::State::STATE_PLAYING);
        }
        else {
            file_chooser(*fileChooserDialog, pipeline, *openFile, *cancelOpen);
        }
    });

    stopButton->signal_clicked().connect([&] {
        pipeline->set_state(Gst::State::STATE_READY);
    });    
    return app->run(*window);
}
