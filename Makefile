all:
	g++ entropist-main.cpp entropist_macos.cpp -o entropist -O2 -std=c++11 -Wall -glldb -framework CoreFoundation -framework ApplicationServices -framework Carbon -lcryptopp