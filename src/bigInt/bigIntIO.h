#ifndef FIBONACCI_BIGINTIO_H

#include "bigInt.h"

#define FIBONACCI_BIGINTIO_H

char *doStoreBigIntInSwap(bigInt *x);

bigInt *doLoadBigIntFromSwap(char *filename);

int writeBigIntToFile(const char *filename, const bigInt *b);

bigInt *readBigIntFromFile(const char *filename);

void removeFile(const char *filename);

bool fileExists(const char *path);

int createDirectory(const char *path);

int directoryExists(const char *path);

#endif //FIBONACCI_BIGINTIO_H
