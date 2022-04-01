head = track.h
srcs = track.cpp
objs = track.o
opts = -w -g -c
all: track
track: $(objs)
	g++ $(objs) -o track
track.o: $(srcs) $(head)
	g++ $(opts) $(srcs)
clean:
	rm track *.o