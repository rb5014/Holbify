compile:
	rm -f holbify
	g++ -o holbify main.cpp `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

run: 
	rm -f holbify
	g++ -o holbify main.cpp `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` 
	./holbify 2<&1 | tee ./holbify.log

test-GUI: 
	rm -f test-holbify
	g++ -o test-holbify main-test.cpp `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0`
	./test-holbify

deux: 
	rm -f test2-holbify
	g++ -o test2-holbify main-test2.cpp  `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0` -I/usr/include/glibmm-2.4
	./test2-holbify

troix:
	rm -f test3
	g++ main-test3.cpp -o test3 `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 gstreamer-app-1.0`
	./test3

quatre:
	rm -f test4-holbify
	g++ -g -std=c++11 main-test4.cpp -o test4-holbify `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0`
	./test4-holbify