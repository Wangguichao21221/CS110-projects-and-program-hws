CC := gcc
CFLAGS := -Wall -O0  -fopenmp  -mavx512f -mavx512dq -mavx512vl
 
SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)

.PHONY: all clean

all:: $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean::
	rm -f *.o