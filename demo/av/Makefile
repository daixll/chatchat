CXX = g++
CXXFLAGS = -std=c++20 -O2 -g

INC_DIR = /home/jiao/Software/FFmpeg-7.0.1/include
LIB_DIR = /home/jiao/Software/FFmpeg-7.0.1/lib

LIBS = -lavformat -lavcodec -lavdevice -lavutil -lswscale -lswresample -lavfilter -lSDL2

LDFLAGS = -Wl,-rpath=$(LIB_DIR)

all: a b

a: a.cpp
	$(CXX) $(CXXFLAGS) a.cpp -I$(INC_DIR) -L$(LIB_DIR) $(LDFLAGS) -o a $(LIBS)

b: b.cpp
	$(CXX) $(CXXFLAGS) b.cpp -I$(INC_DIR) -L$(LIB_DIR) $(LDFLAGS) -o b $(LIBS)

clean:
	rm -f a b mjpg.yuv

.PHONY: all clean