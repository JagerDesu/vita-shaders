CC := arm-linux-gnueabihf-gcc
LD := $(CC)
CFLAGS := -std=gnu99 -Wall -O1 -mfloat-abi=hard -march=armv7-a -mtune=cortex-a9 -mfpu=neon
LDFLAGS := $(CFLAGS)

OBJS := main.o resolve.o syscalls.o

all: shacc

shacc: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

clean:
	-rm -f $(OBJS) shacc
