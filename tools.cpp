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