# ©2017-2018 YUICHIRO NAKADA

PROGRAM = $(patsubst %.c,%,$(wildcard *.c))

CC = clang
CFLAGS = -Os
LDFLAGS = -lasound
LDFLAGS += `pkg-config --libs --cflags OpenCL` -lm
LDFLAGS += `pkg-config --libs --cflags glesv2 egl gbm` -lglfw

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
