head = lab_1.h
srcs = lab_1.cpp
objs = lab_1.o
opts = -g -c
all: lab_1
lab_1: $(objs)
	    gcc $(objs) -o lab_1
lab_1.o: $(srcs) $(head)
		gcc $(opts) $(srcs)
clean:
		rm lab_1 *.o