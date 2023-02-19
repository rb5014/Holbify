#include "functions.h"
#include <cstdlib>
#include <gtkmm.h>
#include <gstreamermm.h>
#include <gst/gst.h>
#include <gst/tag/tag.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <iostream>
#include <memory>
#include <deque>



Glib::RefPtr<Gst::PlayBin> create_playbin() {
    Gst::init();
    return Gst::PlayBin::create("playbin");
}

Glib::RefPtr<Gtk::Application> create_application(int argc, char *argv[]) {
    Glib::init();
    return Gtk::Application::create(argc, argv, "com.example.Holbify");
}

widgets load_widgets(Glib::RefPtr<Gtk::Builder> builder)
{
    widgets w;
    builder->get_widget("mainWindow", w.mainWindow);
    builder->get_widget("openMenuItem", w.openMenuItem);
    builder->get_widget("fileChooserDialog", w.fileChooserDialog);
    builder->get_widget("quitMenuItem", w.quitMenuItem);
    builder->get_widget("playPauseButton", w.playPauseButton);
    builder->get_widget("previousButton", w.previousButton);
    builder->get_widget("forwardButton", w.forwardButton);
    builder->get_widget("stopButton", w.stopButton);
    builder->get_widget("openFile", w.openFile);
    builder->get_widget("cancelOpen", w.cancelOpen);
    builder->get_widget("volumeButton", w.volumeButton);
    builder->get_widget("scaleBar", w.scaleBar);
    builder->get_widget("positionLabel", w.positionLabel);
    builder->get_widget("lengthLabel", w.lengthLabel);
    builder->get_widget("songInfoLabel", w.songInfoLabel);
    return w;
}

#include <gtkmm/button.h>
#include <gtkmm/image.h>

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

    w.forwardButton->set_image(*next_icon);
    w.forwardButton->set_always_show_image(true);
    w.forwardButton->set_tooltip_text("Next");

    w.stopButton->set_image(*stop_icon);
    w.stopButton->set_always_show_image(true);
    w.stopButton->set_tooltip_text("Stop");
}




void connect_signals(widgets& w, const Glib::RefPtr<Gst::PlayBin>& playbin, const Glib::RefPtr<Gtk::Application>& app) {
    w.quitMenuItem->signal_activate().connect([&] {app->quit(); });

    w.openMenuItem->signal_activate().connect(sigc::bind(sigc::ptr_fun(&file_chooser), playbin, std::ref(w)));

    w.playPauseButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_play_pause_button_clicked), playbin, std::ref(w)));

    w.previousButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_previous_button_clicked), playbin, std::ref(w)));

    w.forwardButton->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&on_forward_button_clicked), playbin, std::ref(w)));

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
            return update_scale_bar(playbin, std::ref(w));
        }
        return true;
    }, 100);

}


