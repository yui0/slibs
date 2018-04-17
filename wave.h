#ifndef __WAVE_H_INCLUDED__
#define __WAVE_H_INCLUDED__

#define HEADERSIZE 44

typedef struct {
	signed short l;
	signed short r;
} Soundsample16;

typedef struct {
	unsigned char l;
	unsigned char r;
} Soundsample8;

typedef struct {
	unsigned short channelnum;	// モノラルなら1、ステレオなら2
	unsigned long samplingrate;	// Hz単位
	unsigned short bit_per_sample;	// 1サンプルあたりのbit数
	unsigned long datanum;		// モノラルならサンプル数を、ステレオなら左右１サンプルずつの組の数

	unsigned char *monaural8;	// 8ビットモノラルのデータならこれを使う
	signed short *monaural16;	// 16ビットモノラルならばこれを使う
	Soundsample8 *stereo8;		// 8ビットステレオならばこれを使う
	Soundsample16 *stereo16;	// 16ビットステレオならばこれを使う
} Sound;

// 取得に成功すればポインタを失敗すればNULLを返す
Sound *readWave(char *filename);

// 書き込みに成功すれば0を失敗すれば1を返す
int writeWave(char *filename, Sound *snd);

// Soundを作成し、引数の情報に合わせて領域の確保をする。使われる形式以外の領域のポインタはNULL
// 成功すればポインタを、失敗すればNULLを返す
Sound *createSound(unsigned short channelnum, unsigned long samplingrate, unsigned short bit_per_sample, unsigned long datasize);

// Soundを開放する
void freeSound(Sound *snd);


#include <stdio.h>
#include <malloc.h>
#include <memory.h>

Sound *createSound(unsigned short channelnum, unsigned long samplingrate, unsigned short bit_per_sample, unsigned long datasize)
{
	Sound *snd;

	if ((snd = (Sound *)malloc(sizeof(Sound))) == NULL) {
		fprintf(stderr, "Allocation error\n");
		return NULL;
	}

	snd->channelnum = channelnum;
	snd->samplingrate = samplingrate;
	snd->bit_per_sample = bit_per_sample;
	snd->datanum = datasize / (channelnum*(bit_per_sample/8));

	snd->monaural8 = NULL;
	snd->monaural16 = NULL;
	snd->stereo8 = NULL;
	snd->stereo16 = NULL;

	if (channelnum == 1 && bit_per_sample == 8) {
		if ((snd->monaural8 = (unsigned char *)malloc(datasize)) == NULL) {
			fprintf(stderr, "Allocation error\n");
			free(snd);
			return NULL;
		}
	} else if (channelnum == 1 && bit_per_sample == 16) {
		if ((snd->monaural16 = (signed short *)malloc(sizeof(signed short)*snd->datanum)) == NULL) {
			fprintf(stderr, "Allocation error\n");
			free(snd);
			return NULL;
		}
	} else if (channelnum == 2 && bit_per_sample == 8) {
		if ((snd->stereo8 = (Soundsample8 *)malloc(sizeof(Soundsample8)*snd->datanum)) == NULL) {
			fprintf(stderr, "Allocation error\n");
			free(snd);
			return NULL;
		}
	} else if (channelnum == 2 && bit_per_sample == 16) {
		if ((snd->stereo16 = (Soundsample16 *)malloc(sizeof(Soundsample16)*snd->datanum)) == NULL) {
			fprintf(stderr, "Allocation error\n");
			free(snd);
			return NULL;
		}
	} else {
		fprintf(stderr, "Channelnum or Bit/Sample unknown");
		free(snd);
		return NULL;
	}

	return snd;
}

void freeSound(Sound *snd)
{
	if (snd->channelnum == 1 && snd->bit_per_sample == 8) {
		free(snd->monaural8);
	} else if (snd->channelnum == 1 && snd->bit_per_sample == 16) {
		free(snd->monaural16);
	} else if (snd->channelnum == 2 && snd->bit_per_sample == 8) {
		free(snd->stereo8);
	} else {
		free(snd->stereo16);
	}

	free(snd);
}

Sound *readWave(char *filename)
{
	unsigned int i;
	unsigned char header_buf[20];	// フォーマットチャンクのサイズまでのヘッダ情報を取り込む
	FILE *fp;
	Sound *snd;
	unsigned long datasize;		// 波形データのバイト数
	unsigned short fmtid;		// fmtのIDを格納する
	unsigned short channelnum;	// チャンネル数
	unsigned long samplingrate;	// サンプリング周波数
	unsigned short bit_per_sample;	// 量子化ビット数
	unsigned char *buf;		// フォーマットチャンクIDから拡張部分までのデータを取り込む
	unsigned long fmtsize;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error: %s could not read.", filename);
		return NULL;
	}

	fread(header_buf, sizeof(unsigned char), 20, fp);	// フォーマットチャンクサイズまでのヘッダ部分を取り込む

	// ファイルがRIFF形式であるか
	if (strncmp(header_buf, "RIFF", 4)) {
		fprintf(stderr, "Error: %s is not RIFF.", filename);
		fclose(fp);
		return NULL;
	}

	// ファイルがWAVEファイルであるか
	if (strncmp(header_buf + 8, "WAVE", 4)) {
		fprintf(stderr, "Error: %s is not WAVE.", filename);
		fclose(fp);
		return NULL;
	}

	// fmt のチェック
	if (strncmp(header_buf + 12, "fmt ", 4)) {
		fprintf(stderr, "Error: %s fmt not found.", filename);
		fclose(fp);
		return NULL;
	}

	memcpy(&fmtsize, header_buf + 16, sizeof(fmtsize));

	//printf("%d\n", (sizeof(unsigned char)*fmtsize));//18
	if ((buf = (unsigned char *)malloc(sizeof(unsigned char)*fmtsize)) == NULL) {
		fprintf(stderr, "Allocation error\n");
		fclose(fp);
		return NULL;
	}

	fread(buf, sizeof(unsigned char), fmtsize, fp);		// フォーマットIDから拡張部分までのヘッダ部分を取り込む

	memcpy(&fmtid, buf, sizeof(fmtid));			// LinearPCMファイルならば1が入る

	if (fmtid!=1) {
		// http://www.web-sky.org/program/other/wave.php
		fprintf(stderr, "Error: %s is not LinearPCM. Format is [%d].\n", filename, fmtid);
		fclose(fp);
		return NULL;
	}

	memcpy(&channelnum, buf + 2, sizeof(channelnum));	// チャンネル数を取得
	memcpy(&samplingrate, buf + 4, sizeof(samplingrate));	// サンプリング周波数を取得
	memcpy(&bit_per_sample, buf + 14, sizeof(bit_per_sample)); // 量子化ビット数を取得

	fread(buf, sizeof(unsigned char), 8, fp);		// factもしくはdataのIDとサイズを取得8バイト

	if (!strncmp(buf, "fact", 4)) {
		fread(buf, sizeof(unsigned char), 4, fp);
		fread(buf, sizeof(unsigned char), 8, fp);
	}

	if (strncmp(buf, "data", 4)) {
		do {
			// 余分なチャンクを飛ばす
			int size;
			memcpy(&size, buf + 4, sizeof(size));
//			printf("%c%c%c%c len:%d\n", buf[0], buf[1], buf[2], buf[3], size);
//			fread(buf, sizeof(unsigned char), size, fp);
			fseek(fp, size, SEEK_CUR);
			fread(buf, sizeof(unsigned char), 8, fp);
		} while (strncmp(buf, "data", 4));
//		fprintf(stderr, "Error: %s data part not found.", filename);
//		fclose(fp);
//		return NULL;
	}

	memcpy(&datasize, buf + 4, sizeof(datasize)); //波形データのサイズの取得

	if ((snd = createSound(channelnum, samplingrate, bit_per_sample, datasize)) == NULL) {
		fclose(fp);
		return NULL;
	}

	if (channelnum==1 && bit_per_sample==8) {
		fread(snd->monaural8, sizeof(unsigned char), snd->datanum, fp);		// データ部分を全て取り込む
	} else if (channelnum==1 && bit_per_sample==16) {
		fread(snd->monaural16, sizeof(signed short), snd->datanum, fp);
	} else if (channelnum==2 && bit_per_sample==8) {
		for (i=0; i<snd->datanum; i++) {
			fread(&(snd->stereo8[i].l), sizeof(unsigned char), 1, fp);
			fread(&(snd->stereo8[i].r), sizeof(unsigned char), 1, fp);
		}
	} else if (channelnum==2 && bit_per_sample==16) {
		for (i=0; i<snd->datanum; i++) {
			fread(&(snd->stereo16[i].l), sizeof(signed short), 1, fp);
			fread(&(snd->stereo16[i].r), sizeof(signed short), 1, fp);
		}
	} else {
		fprintf(stderr, "Header is destroyed.");
		fclose(fp);
		freeSound(snd);
	}

	return snd;
}

int writeWave(char *filename, Sound *snd)
{
	int i;
	FILE *fp;
	unsigned char header_buf[HEADERSIZE];	// ヘッダを格納する
	unsigned long fswrh;			// リフヘッダ以外のファイルサイズ
	unsigned long fmtchunksize;		// fmtチャンクのサイズ
	unsigned long dataspeed;		// データ速度
	unsigned short blocksize;		// 1ブロックあたりのバイト数
	unsigned long datasize;			// 周波数データのバイト数
	unsigned short fmtid;			// フォーマットID

	if ((fp = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "Error: %s could not open.", filename);
		return 1;
	}

	fmtchunksize = 16;
	blocksize = snd->channelnum * (snd->bit_per_sample/8);
	dataspeed = snd->samplingrate * blocksize;
	datasize = snd->datanum * blocksize;
	fswrh = datasize + HEADERSIZE - 8;
	fmtid = 1;

	header_buf[0] = 'R';
	header_buf[1] = 'I';
	header_buf[2] = 'F';
	header_buf[3] = 'F';
	memcpy(header_buf + 4, &fswrh, sizeof(fswrh));
	header_buf[8] = 'W';
	header_buf[9] = 'A';
	header_buf[10] = 'V';
	header_buf[11] = 'E';
	header_buf[12] = 'f';
	header_buf[13] = 'm';
	header_buf[14] = 't';
	header_buf[15] = ' ';
	memcpy(header_buf + 16, &fmtchunksize, sizeof(fmtchunksize));
	memcpy(header_buf + 20, &fmtid, sizeof(fmtid));
	memcpy(header_buf + 22, &(snd->channelnum), sizeof(snd->channelnum));
	memcpy(header_buf + 24, &(snd->samplingrate), sizeof(snd->samplingrate));
	memcpy(header_buf + 28, &dataspeed, sizeof(dataspeed));
	memcpy(header_buf + 32, &blocksize, sizeof(blocksize));
	memcpy(header_buf + 34, &(snd->bit_per_sample), sizeof(snd->bit_per_sample));
	header_buf[36] = 'd';
	header_buf[37] = 'a';
	header_buf[38] = 't';
	header_buf[39] = 'a';
	memcpy(header_buf + 40, &datasize, sizeof(datasize));

	fwrite(header_buf, sizeof(unsigned char), HEADERSIZE, fp);

	if (snd->channelnum==1 && snd->bit_per_sample==8) {
		fwrite(snd->monaural8, sizeof(unsigned char), snd->datanum, fp);		// データ部分を全て書き込む
	} else if (snd->channelnum==1 && snd->bit_per_sample==16) {
		fwrite(snd->monaural16, sizeof(signed short), snd->datanum, fp);
	} else if (snd->channelnum==2 && snd->bit_per_sample==8) {
		for (i=0; i<snd->datanum; i++) {
			fwrite(&(snd->stereo8[i].l), sizeof(unsigned char), 1, fp);
			fwrite(&(snd->stereo8[i].r), sizeof(unsigned char), 1, fp);
		}
	} else {
		for (i=0; i<snd->datanum; i++) {
			fwrite(&(snd->stereo16[i].l), sizeof(signed short), 1, fp);
			fwrite(&(snd->stereo16[i].r), sizeof(signed short), 1, fp);
		}
	}

	fclose(fp);

	return 0;
}

#endif /*__WAVE_H_INCLUDED__*/
