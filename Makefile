compile:
	rm -f holbify
	g++ -o holbify main.cpp `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

run: 
	rm -f holbify
	g++ -g -Wall -o holbify main.cpp functions_init.cpp functions_events.cpp functions_tools.cpp -I/usr/include/gstreamer-1.0 `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` -I/usr/include/glibmm-2.4 -lgsttag-1.0
	./holbify 2<&1 | tee ./holbify.log
