#ifndef FIBONACCI_BIGINTIO_H
#define FIBONACCI_BIGINTIO_H

#include "bigInt.h"

char *doStoreBigIntInSwap(bigInt *x);

bigInt *doLoadBigIntFromSwap(char *filename);

int writeBigIntToFile(const char *filename, const bigInt *b);

bigInt *readBigIntFromFile(const char *filename);

void removeFile(const char *filename);

bool fileExists(const char *path);

int createDirectory(const char *path);

int directoryExists(const char *path);

int delete_files_in_folder(const char *folder_path);

#endif //FIBONACCI_BIGINTIO_H
