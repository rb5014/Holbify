compile:
	rm -f holbify
	g++ -o holbify main.cpp `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

run: 
	rm -f holbify
	g++ -g -Wall -o holbify main.cpp functions.cpp -I/usr/include/gstreamer-1.0 `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` -I/usr/include/glibmm-2.4 -lgsttag-1.0
	./holbify 2<&1 | tee ./holbify.log

test-GUI: 
	rm -f test-holbify
	g++ -o test-holbify main-test.cpp `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0`
	./test-holbify

deux: 
	rm -f test2-holbify
	g++ -o test2-holbify main-test2.cpp  `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` -I/usr/include/glibmm-2.4
	./test2-holbify

converted:
	rm -f test2_converted-holbify
	g++ main-test2_converted.cpp -o test2_converted-holbify `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`
	./test2_converted-holbify