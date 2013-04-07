#ifndef BMPLIB_H
#define BMPLIB_H

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} pixel;

int readFile (char[], int *, int *, pixel **);
int writeFile (char[], int, int, pixel *); 

int flip (pixel *, pixel **, int, int);
int enlarge (pixel *, int, int, int, pixel **, int *, int *);


#endif
