CC := arm-linux-gnueabihf-gcc
LD := $(CC)
CFLAGS := -std=gnu99 -g
LDFLAGS := $(CFLAGS)

OBJS := main.o resolve.o syscalls.o

all: shacc

shacc: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

clean:
	-rm -f $(OBJS) shacc