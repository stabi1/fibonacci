#include "bigIntIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <stdatomic.h>
#include "../misc.h"

const char *NOT_STORED = "NOT_STORED";
const char *FILE_ENDING = ".bigint";
const char *SWAP_DIR = "swap_storage/";

uint64_t counter = 0;

void handleSignals(int sig, siginfo_t *info, void *context) {
    (void) info;     // Unused parameter
    (void) context;  // Unused parameter
    if (sig == SIGTERM || sig == SIGINT) {
        printf("\nSIGINT or SIGTERM received, cleaning up resources\n");
        fflush(stdout); // Ensure the output is immediately visible
        if (directoryExists(SWAP_DIR)) {
            int status = delete_files_in_folder(SWAP_DIR);
            if (status == 0) {
                printf("Cleanup successful, exiting\n");
                exit(EXIT_FAILURE);
            } else {
                fprintf(stderr, "Errors occurred during cleanup, exiting\n");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Nothing to clean up, exiting\n");
            exit(EXIT_FAILURE);
        }
    }
}

char *get_16_hex_string() {
    unsigned char *hex_str = malloc(17);
    mallocCheck(hex_str);
    size_t counterLocal = atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed);
    for (int i = 15; i >= 0; --i) {
        uint8_t digit = counterLocal >> (i * 4) & 0xF;
        hex_str[15 - i] = (digit < 10) ? (unsigned char) ('0' + digit) : (unsigned char) ('A' + (digit - 10));
    }
    hex_str[16] = '\0';

    return (char *) hex_str;
}

char *getFilename() {
    size_t endingLen = strlen(FILE_ENDING);
    char *filename;
    while (true) {
        char *file_prefix = get_16_hex_string();
        size_t prefixLen = strlen(file_prefix);
        size_t pathLen = strlen(SWAP_DIR);
        filename = malloc(pathLen + prefixLen + endingLen + 1);
        mallocCheck(filename);
        strncpy(filename, SWAP_DIR, pathLen);
        strncpy(filename + pathLen, file_prefix, prefixLen);
        strncpy(filename + pathLen + prefixLen, FILE_ENDING, endingLen);
        filename[pathLen + prefixLen + endingLen] = '\0';
        free(file_prefix);
        if (!fileExists(filename)) {
            break;
        } else {
            free(filename);
        }
    }
    return filename;
}

//prints bigInt in Binary
void printBigIntBinary(const bigInt *x) {
    char *tmp = bigIntToBinaryString(x, false);
    printf("%s\n", tmp);
    free(tmp);
}

//prints bigInt in Hex
void printBigIntHex(const bigInt *x) {
    char *tmp = bigIntToHexString(x, false);
    printf("%s\n", tmp);
    free(tmp);
}

//prints bigInt in Dec
void printBigIntDec(const bigInt *x) {
    // with doFree=False bigIntToDecString does not modify x, so discard const for the compiler
    char *tmp = bigIntToDecString((bigInt *) x, false);
    printf("%s\n", tmp);
    free(tmp);
}

void writeBigIntHexToFile(const bigInt *x, const char *path) {
    char *tmp = bigIntToHexString(x, false);
    writeFile(path, tmp, false);
    free(tmp);
}

void writeBigIntDecToFile(bigInt *x, const char *path, bool doFree) {
    char *tmp = bigIntToDecString(x, doFree);
    writeFile(path, tmp, false);
    free(tmp);
}

bigInt *readBigIntHexFromFile(const char *path) {
    char *fileContent = readFile(path);
    if (fileContent == NULL) exit(EXIT_FAILURE);
    bigInt *res = hexStringToBigInt(fileContent);
    free(fileContent);
    return res;
}

bigInt *readBigIntDecFromFile(const char *path) {
    char *fileContent = readFile(path);
    if (fileContent == NULL) exit(EXIT_FAILURE);
    bigInt *res = decStringToBigInt(fileContent);
    free(fileContent);
    return res;
}

inline __attribute__((always_inline)) char *storeBigIntInSwap(bigInt *x) {
    if (!global_config.swap || !x->arrayOwner || getLen(x) * 8 < global_config.swapThreshold * 1000000) {
        size_t l = strlen(NOT_STORED);
        char *res = malloc(l + 1);
        mallocCheck(res);
        strncpy(res, NOT_STORED, l + 1);
        return res;
    }
    return doStoreBigIntInSwap(x);
}

inline __attribute__((always_inline)) bigInt *loadBigIntFromSwap(bigInt *x, char *filename) {
    if (strcmp(filename, NOT_STORED) == 0) {
        free(filename);
        return x;
    }
    return doLoadBigIntFromSwap(filename);
}

char *doStoreBigIntInSwap(bigInt *x) {
    //build filename
    char *filename = getFilename();
    //check swap directory
    if (!directoryExists(SWAP_DIR)) {
        int status = createDirectory(SWAP_DIR);
        if (status == -1) {
            exit(EXIT_FAILURE);
        }
    }

    //write to file
    int status = writeBigIntToFile(filename, x);
    if (status == -1) {
        exit(EXIT_FAILURE);
    }
    freeBigInt(x);
    return filename;
}

bigInt *doLoadBigIntFromSwap(char *filename) {
    bigInt *res = readBigIntFromFile(filename);
    if (res == NULL) {
        exit(EXIT_FAILURE);
    }
    removeFile(filename);
    free(filename);
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
        perror("Error reading file");
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
    const size_t stringLen = strlen(string);
    if (fwrite(string, 1, stringLen, file) != stringLen) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

bigInt *readBigIntFromFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Load the primitive fields
    size_t end;
    if (fread(&end, sizeof(size_t), 1, file) != 1) {
        perror("Error reading file");
        fclose(file);
        return NULL;
    }

    bool negative;
    if (fread(&negative, sizeof(bool), 1, file) != 1) {
        perror("Error reading file");
        fclose(file);
        return NULL;
    }

    // Create the bigInt
    size_t arrayLength = end;
    bigInt *res = newBigIntNotZeroed(arrayLength);
    res->negative = negative;

    if (fread(res->bigIntArray, sizeof(uint64_t), arrayLength, file) != arrayLength) {
        perror("Error reading file");
        free(res);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return res;
}

int writeBigIntToFile(const char *filename, const bigInt *b) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    // Save the primitive fields
    size_t length = getLen(b);
    if (fwrite(&length, sizeof(size_t), 1, file) != 1) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }
    if (fwrite(&b->negative, sizeof(bool), 1, file) != 1) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }

    // Save the bigIntArray length and data
    size_t arrayLength = b->end - b->start;
    if (fwrite(b->bigIntArray + b->start, sizeof(uint64_t), arrayLength, file) != arrayLength) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void removeFile(const char *path) {
    if (remove(path) != 0)
        perror("Error deleting file");
}

bool fileExists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

int createDirectory(const char *path) {
    if (mkdir(path, 0755) == 0) {
        return 0;
    } else {
        perror("Error creating directory");
        return -1;
    }
}

int directoryExists(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0)
        return false;

    if (S_ISDIR(info.st_mode)) {
        return true;
    } else {
        // The path exists but is not a directory
        return false;
    }
}

int delete_files_in_folder(const char *folder_path) {
    struct dirent *entry;
    DIR *dir = opendir(folder_path);
    bool error = false;

    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path to the file
        char file_path[4096];
        snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);

        // Attempt to delete the file
        if (unlink(file_path) == -1) {
            perror(file_path);
            error = true;
        }
    }

    closedir(dir);
    return error ? -1 : 0;
}
