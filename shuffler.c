#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bmplib.h"
#include "mt19937int.h"

extern char *optarg;
extern int optind;

int usage()
{
	printf("Usage: bmptool -m <method> -p <piles> -r <reps> "
	                      "-e <error> -n <cards> <outputfile>\n"
	       "\t<method> = 'pile' or 'riffle' or 'random'\n"
	       "\t <piles> = how many piles? default 5 (ignored w/o pile)\n"
	       "\t  <reps> = how many repeats? default 1\n"
	       "\t <error> = error probability (in %%) default 0 (ignored w/o riffle)\n"
	       "\t <cards> = how big is the deck? default 45\n");
	// printf("err at %d\n", line);
	return -1;
}

#define CARD_WIDTH 10
#define CARD_HEIGHT 40

inline void *XCALLOC(size_t size) {
	char *p = calloc(size, 1); assert(p != NULL); return p;
}

enum mode { PILE, RIFFLE, RANDOM };
long piles = 5;
long reps = 1;
long cards = 45;
long error = 0;
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
	pixel *flat = XCALLOC(rows * cols * sizeof(pixel));
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
	int i, j, card;

	int **ps  = XCALLOC(piles * sizeof(int *));
	int *lens = XCALLOC(piles * sizeof(int));
	for (i = 0; i < piles; i++) {
		ps[i] = XCALLOC(cards * sizeof(int));
	}

	// pile
	for (i = 0; i < cards; i++) {
		int num = i % piles;
		ps[num][lens[num]] = deck[i];
		lens[num]++;
	}
	// collate
	card = 0;
#ifdef PILE_FORWARDS
	for (i = 0; i < piles; i++) {
#else
	for (i = piles - 1; i >= 0; i--) { // stack piles backwards
#endif
		for (j = 0; j < lens[i]; j++) {
			deck[card] = ps[i][j];
			card++;
		}
	}
	assert(card == cards);

	// cleanup
	for (i = 0; i < piles; i++) {
		free(ps[i]);
	}
	free(ps);
	free(lens);
}

void shuffle_riffle(int *deck) {
	int midpoint = cards / 2;
	int index[2]    = { 0,        midpoint };
	int maxindex[2] = { midpoint, cards    };
	int swap = 1; // second pile first
	int i;
	int *newdeck = XCALLOC(cards * sizeof(int));

	for (i = 0; i < cards; i++) {
		if (index[swap] == maxindex[swap]) {
			swap = (swap + 1) % 2;
			assert(index[swap] < maxindex[swap]);
		}
		newdeck[i] = deck[index[swap]];
		index[swap]++;
		if ((genrand() % 100) >= error) {
			swap = (swap + 1) % 2;
		} else {
			// printf("shuffle error at index %d\n", index[swap]);
		}
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
	int *deck = XCALLOC(cards * sizeof(int));
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

#define GET_NUM(varp) do { char *endptr; *varp = strtol(optarg, &endptr, 10); if (endptr == optarg) { printf("argument not a number: %s\n", optarg); exit(usage()); } } while (0)

#define MATCH_STR(s) (strncmp(optarg, s, strlen(s)) == 0)

int main(int argc, char * const argv[])
{
	int c;

	while ((c = getopt(argc, argv, "m:p:r:n:e:")) != -1) {
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
			case 'e': GET_NUM(&error); break;
			case '?':
			default:
				exit(usage());
		}
	}
	if ((file = argv[optind]) == NULL) {
		printf("No output file specified.\n");
		exit(usage());
	}
	if (error < 0 || error > 100) {
		printf("Error must be between 0 and 100 (percent)\n");
		exit(usage());
	} else if (cards > 65536) {
		printf("Too many cards (%ld)\n", cards);
		exit(usage());
	}
	sgenrand(time(NULL));
	// printf("piles: %ld, reps: %ld, cards: %ld, mode: %d, file: %s\n", piles, reps, cards, mode, argv[optind]);
	return go();
}
