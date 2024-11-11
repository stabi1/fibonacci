#include "bigInt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//prints bigInt in Hex
void printBigIntHex(bigInt *x) {
    char *tmp = bigIntToHexString(x);
    printf("%s\n", tmp);
    free(tmp);
}

//prints bigInt in Dec
void printBigIntDec(bigInt *x) {
    char *tmp = bigIntToDecString(x);
    printf("%s\n", tmp);
    free(tmp);
}

void writeBigIntHexToFile(bigInt *x, char* path) {
    char *tmp = bigIntToHexString(x);
    writeFile(path, tmp, false);
    free(tmp);
}

void writeBigIntDecToFile(bigInt *x, char* path) {
    char *tmp = bigIntToDecString(x);
    writeFile(path, tmp, false);
    free(tmp);
}

bigInt *readBigIntHexFromFile(char *path) {
    char *fileContent = readFile(path);
    if(fileContent == NULL) exit(EXIT_FAILURE);
    bigInt* res = hexStringToBigInt(fileContent);
    return res;
}

bigInt *readBigIntDecFromFile(char *path) {
    char *fileContent = readFile(path);
    if(fileContent == NULL) exit(EXIT_FAILURE);
    bigInt* res = decStringToBigInt(fileContent);
    return res;
}

char *readFile(const char *path) {
    char *string = NULL;
    FILE *file;
    if (!(file = fopen(path, "r"))) {
        perror("Error opening file");
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fileno(file), &statbuf)) {
        perror("Error retrieving file stats\n");
        goto cleanup;
    }

    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fprintf(stderr, "Error processing file: Not a regular file or invalid size\n");
        goto cleanup;
    }

    if (!(string = malloc(statbuf.st_size + 1))) {
        fprintf(stderr, "Error reading file: Could not allocate enough memory\n");
        goto cleanup;
    }
    if (fread(string, 1, statbuf.st_size, file) != (size_t) statbuf.
            st_size) {
        fprintf(stderr, "Error reading file");
        free(string);
        string = NULL;
        goto cleanup;
    }
    string[statbuf.st_size] = '\0';

    cleanup:
    if (file)
        fclose(file);
    return string;
}

int writeFile(const char *path, const char *string, bool append) {
    FILE *file;
    char *mode = append ? "a" : "w";
    if (!(file = fopen(path, mode))) {
        perror("Error opening file");
        return -1;
    }
    const size_t stringlen = strlen(string);
    if (fwrite(string, 1, stringlen, file) != stringlen) {
        fprintf(stderr, "Error writing to file \n");
        return -1;
    }
    fclose(file);
    return 0;
}
