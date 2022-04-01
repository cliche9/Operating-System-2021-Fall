srcs = exp3.c
objs = exp3.o
opts = -g -c
all: exp3
exp3: $(objs)
	gcc $(objs) -o exp3 -lreadline 
exp3.o: $(srcs)
	gcc $(opts) $(srcs)
clean:
	rm exp3 *.o