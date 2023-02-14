#include <gtkmm.h>
#include <gstreamermm.h>

// This code uses decodebin
int main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    Glib::RefPtr<Gtk::Application> app =
        Gtk::Application::create(argc, argv, "com.example.Holbify");

    Gtk::Window window;
    window.set_default_size(200, 200);

    Gtk::Button button("Play");
    window.add(button);

    Glib::RefPtr<Gst::Pipeline> pipeline =
        Gst::Pipeline::create("my-pipeline");

    Glib::RefPtr<Gst::Element> src =
        Gst::ElementFactory::create_element("filesrc");
    std::string file_path("/home/rb5014/Holbify/1.mp3");
    src->set_property("location", file_path);

    Glib::RefPtr<Gst::Element> decoder =
        Gst::ElementFactory::create_element("decodebin");

    Glib::RefPtr<Gst::Element> sink =
        Gst::ElementFactory::create_element("autoaudiosink");

    pipeline->add(src)->add(decoder)->add(sink);
    src->link(decoder);

    // Connect to the "pad-added" signal of the decodebin element to link it to the sink
    decoder->signal_pad_added().connect([&](const Glib::RefPtr<Gst::Pad> &pad) {
        Glib::RefPtr<Gst::Pad> sink_pad = sink->get_static_pad("sink");
        pad->link(sink_pad);
    });

    button.signal_clicked().connect([&] {
        pipeline->set_state(Gst::State::STATE_PLAYING);
    });

    window.show_all();
    return app->run(window);
}