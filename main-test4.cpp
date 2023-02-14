#include <gtkmm.h>
#include <gstreamermm.h>
#include <cstdlib>
#include <iostream>
#include <glibmm/ustring.h>
#include <memory>
#include <functional>

// Forward declarations
namespace Gtk {
    class Scale;
    class FileChooserDialog;
    class Button;
    class VolumeButton;
}

namespace Gst {
    class PlayBin;
}



bool update_scale_bar(Gtk::Scale& scaleBar, Glib::RefPtr<Gst::PlayBin> playbin)
{
    gint64 pos, len;
    Gst::Format format = Gst::FORMAT_TIME;

    // Get the current position and length of the song in time format
    if (playbin->query_position(format, pos) && playbin->query_duration(format, len))
    {
        // Calculate the percentage of the song that has been played
        double value = static_cast<double>(pos) / static_cast<double>(len);

        // Set the range of the scale bar to the length of the song
        scaleBar.set_range(0, len / GST_SECOND);

        // Format the position as minutes and seconds
        int pos_min = (pos / GST_SECOND) / 60;
        int pos_sec = (pos / GST_SECOND) % 60;

        // Update the value and tooltip text of the scale bar
        scaleBar.set_value(pos / GST_SECOND);
        scaleBar.set_tooltip_text(Glib::ustring::compose("%1:%2", pos_min, pos_sec));
        scaleBar.set_digits(0);
    }

    return true;
}

void on_scaleBar_value_changed(Gtk::Scale* scaleBar, Glib::RefPtr<Gst::PlayBin> playbin)
{
    gint64 len = 0;
    playbin->query_duration(Gst::FORMAT_TIME, len);
    gint64 pos = scaleBar->get_value() * GST_SECOND;

    // Seek to the new position in the song
    playbin->seek_simple(Gst::FORMAT_TIME, Gst::SeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), pos);

    // Update the tooltip text of the scale bar
    int pos_min = (pos / GST_SECOND) / 60;
    int pos_sec = (pos / GST_SECOND) % 60;
    scaleBar->set_tooltip_text(Glib::ustring::compose("%1:%2", pos_min, pos_sec));
}

void file_opener(Glib::ustring& filename, Glib::RefPtr<Gst::PlayBin> playbin) {
    playbin->property_uri() = Glib::filename_to_uri(filename);
    playbin->set_state(Gst::STATE_PLAYING);
}

void file_chooser(Gtk::FileChooserDialog* dialog,
                  Glib::RefPtr<Gst::PlayBin> playbin,
                  Gtk::Button& openFile,
                  Gtk::Button& cancelOpen)
{
    openFile.signal_clicked().connect([&dialog]{
        dialog->response(Gtk::RESPONSE_OK);
    });

    cancelOpen.signal_clicked().connect([&dialog]{
        dialog->response(Gtk::RESPONSE_CANCEL);
    });

    int result = dialog->run();

    // Handle the result of the file chooser dialog
    if (result == Gtk::RESPONSE_OK) {
        Glib::ustring filename = dialog->get_filename();
        playbin->set_state(Gst::STATE_NULL);
        file_opener(filename, playbin);
    }
    else if (result == Gtk::RESPONSE_CANCEL) {
        dialog->hide();
    }

    dialog->hide();
}


void connect_signals(Glib::RefPtr<Gst::PlayBin> playbin,
                     Gtk::FileChooserDialog* fileChooserDialog,
                     Gtk::Button& openFile,
                     Gtk::Button& cancelOpen,
                     Gtk::Scale& scaleBar,
                     Gtk::Button& playPauseButton,
                     Gtk::Button& stopButton,
                     Gtk::VolumeButton& volumeButton,
                     Glib::RefPtr<Gtk::Application> app,
                     Gtk::Window& window,
                     Gtk::MenuItem& quitMenuItem,
                     Gtk::MenuItem& openMenuItem)
{
    /* Connect the Menu items */

    // Connect the Quit menu item to the quit signal
    quitMenuItem.signal_activate().connect([&app] {
        app->quit();
    });

    // Connect the Open menu item to the FileChooserDialog opening by using the function on_open_menu_activated
    openMenuItem.signal_activate().connect([&] {file_chooser(fileChooserDialog, playbin, openFile, cancelOpen);});

    playPauseButton.signal_clicked().connect([playbin, fileChooserDialog, &openFile, &cancelOpen] {
        Gst::State current, pending;
        playbin->get_state(current, pending, 0);
        if (current == Gst::State::STATE_PLAYING) {
            playbin->set_state(Gst::State::STATE_PAUSED);
        }
        else if (current == Gst::State::STATE_PAUSED || current == Gst::State::STATE_READY) {
            playbin->set_state(Gst::State::STATE_PLAYING);
        }
        else {
            file_chooser(fileChooserDialog, playbin, openFile, cancelOpen);
        }
    });

    stopButton.signal_clicked().connect([playbin] {
        playbin->set_state(Gst::State::STATE_READY);
    });

    volumeButton.signal_value_changed().connect([playbin, &volumeButton] (const double &value) {
        double volume = volumeButton.get_value();
        playbin->property_volume() = volume;
    });


    // Connect the update_scale_bar function to the timeout signal of the GTK main loop
    Glib::signal_timeout().connect([&]() {
        update_scale_bar(scaleBar, playbin);
        return true;
    }, 100);


    // Show the window
    window.show_all();
}



int main(int argc, char *argv[])
{
    Glib::init();
    Gst::init(argc, argv);

    // Create the app
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.example.Holbify");

    // Create the playbin
    Glib::RefPtr<Gst::PlayBin> playbin = Gst::PlayBin::create("playbin");

    // Load the Glade XML file
     Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("GUI.glade");

    // Get the widgets from the Glade XML file
    Gtk::Window* mainWindow;
    Gtk::MenuItem* openMenuItem;
    Gtk::FileChooserDialog* fileChooserDialog;
    Gtk::MenuItem* quitMenuItem;
    Gtk::Button* playPauseButton;
    Gtk::Button* stopButton;
    Gtk::Button* openFile;
    Gtk::Button* cancelOpen;
    Gtk::VolumeButton* volumeButton;
    Gtk::Scale* scaleBar;

    builder->get_widget("mainWindow", mainWindow);
    builder->get_widget("openMenuItem", openMenuItem);
    builder->get_widget("fileChooserDialog", fileChooserDialog);
    builder->get_widget("quitMenuItem", quitMenuItem);
    builder->get_widget("playPauseButton", playPauseButton);
    builder->get_widget("stopButton", stopButton);
    builder->get_widget("openFile", openFile);
    builder->get_widget("cancelOpen", cancelOpen);
    builder->get_widget("volumeButton", volumeButton);
    builder->get_widget("scaleBar", scaleBar);

    connect_signals(playbin, fileChooserDialog, *openFile, *cancelOpen, *scaleBar, *playPauseButton, *stopButton, *volumeButton, app, *mainWindow, *openMenuItem, *quitMenuItem);

    return app->run(*mainWindow);
}
