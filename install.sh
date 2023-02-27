#!/bin/bash

# Install the required libraries
sudo apt-get update
sudo apt-get install -y libgtkmm-3.0-dev libgstreamermm-1.0-dev libglibmm-2.4-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-ugly1.0-dev

# Build the holbify program
g++ -g -Wall -o holbify main.cpp functions_init.cpp functions_events.cpp functions_tools.cpp -I/usr/include/gstreamer-1.0 `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` -I/usr/include/glibmm-2.4 -lgsttag-1.0

# Run the holbify program
./holbify
