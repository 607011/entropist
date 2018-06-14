all:
	g++ entropist.cpp -o entropist -O2 -std=c++11 -Wall -ggdb -framework CoreFoundation -framework ApplicationServices -framework Carbon -lcryptopp