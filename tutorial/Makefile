temporal: temporal.cpp heap.cpp
	g++ temporal.cpp heap.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o temporal -std=c++11 -Wall -Werror


ssd: ssd.cpp
	g++ ssd.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o ssd -std=c++11 -Wall -Werror
