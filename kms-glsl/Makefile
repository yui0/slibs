# ©2022 YUICHIRO NAKADA

PROGRAM = $(patsubst %.c,%,$(wildcard *.c))

ifneq (, $(shell which clang))
CC = clang
endif
ifneq (, $(shell which icc))
CC = icc
endif
CFLAGS = -I../ -Os -ffunction-sections -fdata-sections -funroll-loops -finline-functions -ftree-vectorize -pthread -I/usr/include/libdrm
LDFLAGS = -Wl,-s -Wl,--gc-sections -Wl,--as-needed -Wl,--no-undefined
LDLIBS = -lGLESv2 -lEGL -ldrm -lgbm

.PHONY: all
all: depend $(PROGRAM)

%.o : %.c $(HEAD)
	$(CC) $(LDFLAGS) $(CFLAGS) -c $(@F:.o=.c) -o $@ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) _depend.inc

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) _depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> _depend.inc; done

-include _depend.inc
