#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "bmplib.h"

extern char *optarg;
extern int optind, opterr, optopt;

int usage();

#if 0
static pixel *image;
static pixel *new;
static int rows;
static int cols;
#endif

#define usage() print_usage(__LINE__)

int print_usage(int __attribute__((unused)) line)
{
	printf("Usage: bmptool -m <method> -p <piles> -r <reps> "
	                      "-n <cards> <outputfile>\n"
	       "\t<method> = 'pile' or 'riffle' or 'random'\n"
	       "\t <piles> = how many piles? default 5 (ignored w/o pile)\n"
	       "\t  <reps> = how many repeats? default 1\n"
	       "\t <cards> = how big is the deck? default 45\n");
	// printf("err at %d\n", line);
	return -1;
}

#define CARD_WIDTH 10
#define CARD_HEIGHT 40

inline void *XMALLOC(size_t size) {
	char *p = malloc(size); assert(p != NULL); return p;
}

enum mode { PILE, RIFFLE, RANDOM };
long piles = 5;
long reps = 1;
long cards = 45;
enum mode mode = RIFFLE;
char *file;

int hue_to_red(int hue) {
	assert(hue >= 0 && hue < 360);
	if (hue <= 60)       return 255;
	else if (hue <= 120) return ((120 - hue) * 255)/60;
	else if (hue <= 240) return 0;
	else if (hue <= 300) return ((hue - 240) * 255)/60;
	else                 return 255;
}

pixel colorize(int hue) {
	pixel x;
	x.blue  = hue_to_red(hue); // XXX: why are they backwards?
	x.green = hue_to_red((hue + 240) % 360);
	x.red   = hue_to_red((hue + 120) % 360); // as above??
	return x;
}

#define ROWS() CARD_HEIGHT
#define COLS()  (cards * CARD_WIDTH)
pixel *flatten(int *deck) {
	int rows = ROWS();
	int cols = COLS();
	pixel *flat = XMALLOC(rows * cols * sizeof(pixel));
	int i, j;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			pixel *pp = &flat[(i * cols) + j];
			*pp = colorize(deck[j / CARD_WIDTH]);
			if (!(pp->red > 0 || pp->green > 0 || pp->blue > 0)) {
				printf("Pixel %d,%d (#%d) failed\n", i,j, j/CARD_WIDTH);
			}
		}
	}
	return flat;
}

void shuffle_pile(int *deck) {
	assert(0 && "Unimplemented shuffle mode");
}

void shuffle_riffle(int *deck) {
	int *newdeck = XMALLOC(cards * sizeof(int));
	int i;
	int midpoint = cards / 2;// (cards % 2 == 0) ? (cards / 2) : (cards / 2) + 1;
	for (i = 0; i < midpoint; i++) {
		int index = 1+(i*2);
		assert(index < cards);
		newdeck[index] = deck[i];
	}
	for (i = midpoint; i < cards; i++) {
		int index = (i-midpoint)*2;
		assert(index < cards);
		newdeck[index] = deck[i];
	}
	for (i = 0; i < cards; i++) {
		deck[i] = newdeck[i];
	}
	free(newdeck);
}

void shuffle_random(int *deck) {
	assert(0 && "Unimplemented shuffle mode");
}

int go() {
	int i;
	int *deck = XMALLOC(cards * sizeof(int));
	int ret;
	pixel *image;
	/* init deck */
	for (i = 0; i < cards; i++) {
		deck[i] = i * 360 / cards;
	}
	/* shuffle */
	for (i = 0; i < reps; i++) {
		switch (mode) {
			case PILE:   shuffle_pile  (deck); break;
			case RIFFLE: shuffle_riffle(deck); break;
			case RANDOM: shuffle_random(deck); break;
			default:     assert(0);
		}
	}
	image = flatten(deck);
	free(deck);
	ret = writeFile(file, ROWS(), COLS(), image);
	free(image);
	return ret;
}

/******************************************************************************/

#define GET_NUM(varp) do { char *endptr; *varp = strtol(optarg, &endptr, 0); if (endptr == optarg) { printf("argument not a number: %s\n", optarg); exit(usage()); } } while (0)

#define MATCH_STR(s) (strncmp(optarg, s, strlen(s)) == 0)

int main(int argc, char * const argv[])
{
	int c;

	while ((c = getopt(argc, argv, "m:p:r:n:")) != -1) {
		switch (c) {
			case 'm':
				if      (MATCH_STR("riffle")) mode = RIFFLE;
				else if (MATCH_STR("pile"))   mode = PILE;
				else if (MATCH_STR("random")) mode = RANDOM;
				else                          exit(usage());
				break;
			case 'p': GET_NUM(&piles); break;
			case 'r': GET_NUM(&reps);  break;
			case 'n': GET_NUM(&cards); break;
			case '?':
			default:
				exit(usage());
		}
	}
	if ((file = argv[optind]) == NULL) {
		printf("No output file specified.\n");
		exit(usage());
	}
	printf("piles: %ld, reps: %ld, cards: %ld, mode: %d, file: %s\n",
	       piles, reps, cards, mode, argv[optind]);
	return go();
}
