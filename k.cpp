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

void on_message(Glib::RefPtr<Gst::Message> message, Glib::RefPtr<Gst::PlayBin> playbin, widgets& w, icons i) {
    auto msgType = message->get_message_type();
    //g_print("Received message of type %s\n", gst_message_type_get_name(msgType));
    if (msgType == Gst::MESSAGE_STATE_CHANGED) {
        GstState newState;
        gst_message_parse_state_changed(message->gobj(), nullptr, &newState, nullptr);
        if (newState == GST_STATE_READY || newState == GST_STATE_PAUSED) {
            if (newState == GST_STATE_READY) {
                reset_visuals(std::ref(w));
            }
        } else if (newState == GST_STATE_PLAYING) {
        }
    }
    else if (msgType == Gst::MESSAGE_TAG) {
        auto tag_message = Glib::RefPtr<Gst::MessageTag>::cast_static(message);
        if (tag_message) {
            Gst::TagList tag_list = tag_message->parse_tag_list();
            gchar *title;
            if (gst_tag_list_get_string(tag_list.gobj(), GST_TAG_TITLE, &title)) {
                w.mainWindow->set_title(Glib::ustring(title) + " - Music Player Holbify");
            } else {
                w.mainWindow->set_title(Gio::File::create_for_path(currentSong)->get_basename() + " - Music Player Holbify");
            }
        }
    }
}