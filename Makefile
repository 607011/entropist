all:
	g++ entropist.cpp -o entropist -Wall -ggdb -framework CoreFoundation -framework ApplicationServices -framework Carbon -lcryptopp