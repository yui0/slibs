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

.PHONY: gpu
gpu: catseye_llm.h llama2.c
	gcc -o llama2 llama2.c -DGPGPU -Ofast -fopenmp -march=native -mfpmath=both -mavx -ffast-math -funroll-loops -lm -lglfw -lGL

.PHONY: emscripten
emscripten: llama2.c
	emcc -Oz llama2.c \
	-o web/src/llama2.js \
	-s EXPORTED_FUNCTIONS='["_main", "_main_loop", "_malloc", "_free", "_register_callback", "_set_parameters", "_generate", "_manual_start", "_manual_next", "_get_vocab", "_get_vocab_size"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall", "addFunction", "UTF8ToString"]' \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ALLOW_TABLE_GROWTH=1 \
	-s MODULARIZE \
	-s EXPORT_NAME='Llama2' \
	--preload-file models/tokenizer.bin

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) _depend.inc

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) _depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> _depend.inc; done

-include _depend.inc
