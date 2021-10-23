srcs = lab_3.c
objs = lab_3.o
opts = -g -c
all: lab_3
lab_3: $(objs)
	gcc $(objs) -o lab_3
lab_3.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm lab_3 *.o