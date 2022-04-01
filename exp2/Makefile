srcs = exp2.c
objs = exp2.o
opts = -g -c
all: exp2
exp2: $(objs)
	gcc $(objs) -l pthread -o exp2
exp2.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm exp2 *.o