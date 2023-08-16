# ©2023 YUICHIRO NAKADA

define MAKECMD
gcc -O3 -DNDEBUG -march=native -mavx -mf16c -msse3 -ffast-math -fno-omit-frame-pointer -c ggml.c
g++ -o sd -O3 main.cpp stable-diffusion.cpp ggml.o
endef
export MAKECMD

.PHONY: all
all::
	echo "$${MAKECMD}" > /tmp/$$$$ ; $(SHELL) /tmp/$$$$ ; rm -f /tmp/$$$$

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) _depend.inc

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) _depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> _depend.inc; done

-include _depend.inc
