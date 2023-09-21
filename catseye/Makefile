# ©2023 YUICHIRO NAKADA

define MAKECMD
#PGO optimizations
echo gcc -o llama2 -Ofast -fopenmp -march=native -mfpmath=both -mavx -ffast-math -funroll-loops llama2.c -lm
gcc -o llama2 -Ofast -fopenmp -march=native -mfpmath=both -mavx -ffast-math -funroll-loops llama2.c -lm
endef
export MAKECMD

.PHONY: all
all::
	@echo "$${MAKECMD}" > /tmp/$$$$ ; $(SHELL) /tmp/$$$$ ; rm -f /tmp/$$$$

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) _depend.inc

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) _depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> _depend.inc; done

-include _depend.inc
