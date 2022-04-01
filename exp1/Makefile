head = exp1.h
srcs = exp1.cpp
objs = exp1.o
opts = -g -c
all: exp1
exp1: $(objs)
	    gcc $(objs) -o exp1
exp1.o: $(srcs) $(head)
		gcc $(opts) $(srcs)
clean:
		rm exp1 *.o