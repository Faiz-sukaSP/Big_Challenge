#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <time.h>

// Menyimpan hasil pencarian dan pengurutan kosakata
typedef struct CachedResult
{
  char **words;     // array kata
  long long *freqs; // array frekuensi kata
  int count;        // jumlah elemen yang valid dalam cache
  int k;            // nilai k yang digunakan saat pencarian
  double elapsed;   // waktu yang dibutuhkan untuk heapsort (ms)
  int isValid;      // penanda apakah cache berisi data yang valid (1) atau tidak (0)
} CachedResult;

void sortDuration(clock_t start, clock_t end);
void extractCollectionName(const char *path, char *outName, size_t maxLen);
int generateOutputFile(CachedResult *freqRes, CachedResult *rareRes, const char *collection);
void normalizePaths(const char *inputPath, char *docwordPath, char *vocabPath, size_t maxLen);
void freeCache(CachedResult *cache);

#endif