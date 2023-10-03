//---------------------------------------------------------
//	Cat's eye
//
//		©2016-2023 Yuichiro Nakada
//---------------------------------------------------------

#include <stdlib.h>
#include <ctype.h>
#include "catseye_llama2.h"

void error_usage(char *cmd)
{
	fprintf(stderr, "Usage:   %s <checkpoint> [options]\n", cmd);
	fprintf(stderr, "Example: %s -c model.bin -n 256 -p \"Once upon a time\"\n", cmd);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -m <string> model file\n");

	fprintf(stderr, "  -c <string> checkpoint file\n");
	fprintf(stderr, "  -z <string> optional path to custom tokenizer\n");

	fprintf(stderr, "  -t <float>  temperature in [0,inf], default 1.0\n");
	fprintf(stderr, "  -x <float>  p value in top-p (nucleus) sampling in [0,1], default 0.9\n");
	fprintf(stderr, "  -s <int>    random seed, default time(NULL)\n");
	fprintf(stderr, "  -n <int>    number of steps to run for, default 256. 0 = max_seq_len\n");
	fprintf(stderr, "  -p <string> input prompt\n");

	fprintf(stderr, "  -a <string> mode: generate|chat, default: generate\n");
	fprintf(stderr, "  -y <string> (optional) system prompt in chat mode\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	// default parameters
	char *model = NULL;
	char *checkpoint_path = "models/model.bin";
	char *tokenizer_path = "models/tokenizer.bin";
	char *prompt = NULL;		// prompt string
	int steps = 256;		// max number of steps to run for, 0: use seq_len
	float temperature = 1.0;	// 0.0 = greedy deterministic. 1.0 = original. don't set higher
	float topp = 0.9;		// top-p in nucleus sampling. 1.0 = off. 0.9 works well, but slower
	char *mode = "generate";	// generate|chat
	char *system_prompt = NULL;	// the (optional) system prompt to use in chat mode
	rng_seed = 0;			// seed rng with time by default
	//srand((unsigned int)time(NULL));

	for (int i=1; i<argc; i++) {
		switch (argv[i][0]) {
		case '-':
			switch (argv[i][1]) {
			case 'm': if (i+1 < argc) { model = argv[++i]; }		break;

			case 'c': if (i+1 < argc) { checkpoint_path = argv[++i]; }	break;
			case 'z': if (i+1 < argc) { tokenizer_path = argv[++i]; }	break;

			case 't': if (i+1 < argc) { temperature = atof(argv[++i]); }	break;
			case 'x': if (i+1 < argc) { topp = atof(argv[++i]); }		break;
			case 's': if (i+1 < argc) { rng_seed = atoi(argv[++i]); }	break;
			case 'n': if (i+1 < argc) { steps = atoi(argv[++i]); }		break;
			case 'p': if (i+1 < argc) { prompt = argv[++i]; }		break;

			case 'a': if (i+1 < argc) { mode = argv[++i]; }			break;
			case 'y': if (i+1 < argc) { system_prompt = argv[++i]; }	break;
			default: fprintf(stderr,  "Invalid option: %s\n", argv[i]);	return 1;
			} break;
		default:
			error_usage(argv[0]);
		}
	}
	if (checkpoint_path == NULL) {
		fprintf(stderr, "Error: checkpoint file (model) not set. \nSet with %s -c <checkpoint_file>\n", argv[0]);
		return 1;
	}
	// parameter validation/overrides
	if (rng_seed <= 0) rng_seed = (unsigned int)time(NULL);
	if (temperature < 0.0) temperature = 0.0;
	if (topp < 0.0 || 1.0 < topp) topp = 0.9;
	if (steps < 0) steps = 0;

	// read weight file
	cats_ggml_model m;
	if (model) {
		if (strstr(model, ".gguf")) cats_gguf_load(model, &m); //m.seq_len = 2048;
		else cats_ggml_load(model, &m);
		m.max_token_length = 512;
//		m.max_token_length = 4096;
	} else {
		cats_checkpoint_load(checkpoint_path, &m);

		// read in the tokenizer.bin file
		m.vocab = (char**)malloc(m.n_vocab * sizeof(char*));
		m.score = (float*)malloc(m.n_vocab * sizeof(float));
		{
			int fd = open(tokenizer_path, O_RDONLY);
			if (fd==-1) {
				fprintf(stderr, "Couldn't open file tokenizer.bin\n");
				return 1;
			}
			read(fd, &m.max_token_length, sizeof(uint32_t));
//			printf("max token length: %d\n", m.max_token_length); // 27
			cats_read_vocab(fd, m.n_vocab, m.vocab, m.score, 1);
			close(fd);
		}
	}
	malloc_run_state(&m);

	printf("transformer dimension: %d\n", m.n_embd);
	printf("ffn layers's dimension: %d\n", m.n_hidden);
	printf("number of layers: %d\n", m.n_layer);
	printf("number of query heads: %d\n", m.n_head);
	printf("number of key/value heads: %d\n", m.n_kv_head);
	printf("vocabulary size: %d\n", m.n_vocab);
	printf("max sequence length: %d\n", m.seq_len);
	printf("seed: %d\n\n", rng_seed);

	// right now we cannot run for more than seq_len steps
	if (steps <= 0 || steps > m.seq_len) steps = m.seq_len;

	// run!
	if (strcmp(mode, "generate") == 0) {
		generate(&m, prompt, steps, temperature, topp);
	} else if (strcmp(mode, "chat") == 0) {
		chat(&m, prompt, system_prompt, steps, temperature, topp);
	} else {
		fprintf(stderr, "unknown mode: %s\n", mode);
		error_usage(argv[0]);
	}

	// memory and file handles cleanup
	free_run_state(&m);
	for (int i=0; i<m.n_vocab; i++) {
		free(m.vocab[i]);
	}
	free(m.vocab);
	free(m.score);

	free(m.token_embedding_table->data);
	free(m.rms_final_weight->data);
//	free(m.wcls->data);
	for (int i=0; i<m.n_layer; i++) {
		free(m.rms_att_weight[i]->data);
		free(m.wq[i]->data);
		free(m.wk[i]->data);
		free(m.wv[i]->data);
		free(m.wo[i]->data);
		free(m.rms_ffn_weight[i]->data);
		free(m.w1[i]->data);
		free(m.w2[i]->data);
		free(m.w3[i]->data);
	}

	return 0;
}
