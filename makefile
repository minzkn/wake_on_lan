###
### Copyright (C) HWPORT.COM
### All rights reserved.
### Author: JAEHYUK CHO <mailto:minzkn@minzkn.com>
###

CROSS_COMPILE                ?=#

CC                           :=$(CROSS_COMPILE)gcc#
RM                           :=rm -f#

THIS_NAME                    :=wake_on_lan#

CFLAGS                       :=#
CFLAGS                       +=-Os -pipe#
CFLAGS                       +=-fPIC#
CFLAGS                       +=-fomit-frame-pointer#
CFLAGS                       +=-ansi#
CFLAGS                       +=-Wall -W#
CFLAGS                       +=-Wshadow -Wcast-qual -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline -Wwrite-strings#

LDFLAGS                      :=#

TARGET                       :=$(THIS_NAME)#
OBJECTS                      :=main.o#

.PHONY: all clean

all: $(TARGET)
clean: ;$(RM) *.o $(TARGET)
$(TARGET): $(OBJECTS) ;$(CC) $(LDFLAGS) -o $(@) $(^)
%.o: %.c ;$(CC) $(CFLAGS) -c -o $(@) $(<)

# End of makefile
