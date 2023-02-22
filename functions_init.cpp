#include "functions.h"

// Create and initiate the playbin
Glib::RefPtr<Gst::PlayBin> create_playbin() {
    Gst::init();
    return Gst::PlayBin::create("playbin");
}

// Create and initiate the application
Glib::RefPtr<Gtk::Application> create_application(int argc, char *argv[]) {
    Glib::init();
    return Gtk::Application::create(argc, argv, "com.example.Holbify");
}


// Load the widgets from the GLADE file
widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder)
{
    widgets w;
    builder->get_widget("mainWindow", w.mainWindow);
    w.mainWindow->set_default_size(800, 200);

    builder->get_widget("openMenuItem", w.openMenuItem);
    builder->get_widget("quitMenuItem", w.quitMenuItem);
    builder->get_widget("createPlaylistButton", w.createPlaylistButton);
    builder->get_widget("openPlaylistButton", w.openPlaylistButton);
    builder->get_widget("fileChooserDialog", w.fileChooserDialog);
    builder->get_widget("createPlaylistDialog", w.createPlaylistDialog);
    builder->get_widget("fileListBox", w.fileListBox);
    builder->get_widget("playPauseButton", w.playPauseButton);
    builder->get_widget("previousButton", w.previousButton);
    builder->get_widget("nextButton", w.nextButton);
    builder->get_widget("stopButton", w.stopButton);
    builder->get_widget("openFile", w.openFile);
    builder->get_widget("cancelOpen", w.cancelOpen);
    builder->get_widget("volumeButton", w.volumeButton);
    w.volumeButton->set_value(1.0);
    builder->get_widget("scaleBar", w.scaleBar);
    builder->get_widget("positionLabel", w.positionLabel);
    builder->get_widget("lengthLabel", w.lengthLabel);
    return w;
}

// Load the icons from the icons directory
void load_icons(widgets& w)
{
    auto play_icon = Gtk::manage(new Gtk::Image("icons/play.png"));
    auto prev_icon = Gtk::manage(new Gtk::Image("icons/previous.png"));
    auto next_icon = Gtk::manage(new Gtk::Image("icons/next.png"));
    auto stop_icon = Gtk::manage(new Gtk::Image("icons/stop.png"));

    w.playPauseButton->set_image(*play_icon);
    w.playPauseButton->set_always_show_image(true);
    w.playPauseButton->set_tooltip_text("Play");

    w.previousButton->set_image(*prev_icon);
    w.previousButton->set_always_show_image(true);
    w.previousButton->set_tooltip_text("Previous");

    w.nextButton->set_image(*next_icon);
    w.nextButton->set_always_show_image(true);
    w.nextButton->set_tooltip_text("Next");

    w.stopButton->set_image(*stop_icon);
    w.stopButton->set_always_show_image(true);
    w.stopButton->set_tooltip_text("Stop");
}

// Connect the widgets to their respective callback function
void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app) {
    w.quitMenuItem->signal_activate().connect([&] {app->quit(); });

    w.openMenuItem->signal_activate().connect(sigc::bind(sigc::ptr_fun(&file_chooser), playbin, std::ref(w)));

    w.createPlaylistButton->signal_activate().connect(sigc::ptr_fun(&on_create_playlist_button_clicked));
   
    w.openPlaylistButton->signal_activate().connect(sigc::bind(sigc::ptr_fun(&on_open_playlist_button_clicked), playbin, w));

    w.playPauseButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_play_pause_button_clicked), playbin, std::ref(w)));

    w.previousButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_previous_button_clicked), playbin, std::ref(w)));

    w.nextButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_next_button_clicked), playbin, std::ref(w)));

    w.stopButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_stop_button_clicked), playbin, std::ref(w)));

    w.volumeButton->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_volume_value_changed), playbin));

    w.scaleBar->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&on_scaleBar_value_changed), playbin, std::ref(w)));


    Glib::RefPtr<Gst::Bus> bus = playbin->get_bus();
    bus->add_signal_watch();
    bus->signal_message().connect(sigc::bind(sigc::ptr_fun(on_message), playbin, std::ref(w)));

    Glib::signal_timeout().connect([&]() {
        Gst::State state, pending_state;
        playbin->get_state(state, pending_state, 0);
        if (state == Gst::State::STATE_PLAYING) {
            return on_timeout_update_scale_bar(playbin, std::ref(w));
        }
        return true;
    }, 100);

}


