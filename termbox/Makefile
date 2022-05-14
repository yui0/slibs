# ©2017-2022 YUICHIRO NAKADA

PROGRAM = $(patsubst %.c,%,$(wildcard *.c))

ifneq (, $(shell which clang))
CC = clang
endif
ifneq (, $(shell which icc))
CC = icc
endif
CFLAGS = -I../ -Os -ffunction-sections -fdata-sections -funroll-loops -finline-functions -ftree-vectorize
#CFLAGS = -Os -ffunction-sections -fdata-sections -funroll-loops -finline-functions -ftree-vectorize -march=native
LDFLAGS = -Wl,-s -Wl,--gc-sections -lm
#LDFLAGS = -lasound -lm -Wl,-s -Wl,-dead_strip

.PHONY: all
all: depend $(PROGRAM)

%.o : %.c $(HEAD)
	$(CC) $(LDFLAGS) $(CFLAGS) -c $(@F:.o=.c) -o $@

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) _depend.inc

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) _depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> _depend.inc; done

-include _depend.inc
