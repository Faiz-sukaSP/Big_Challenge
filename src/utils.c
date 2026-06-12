#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Menghitung waktu pengurutan dan mencetak hasil
void sortDuration(clock_t start, clock_t end) {
  double durationMs = ((double)(end - start) / CLOCKS_PER_SEC) *
                      1000.0; // menghitung selisih waktu

  // cetak dalam satuan milidetik
  if (durationMs < 1000.0)
    printf("Waktu pengurutan: %.2f ms\n", durationMs);

  // cetak dalam satuan detik
  else {
    double durationSec =
        durationMs /
        1000.0; // jika 1 detik atau lebih, konversi ke detik terlebih dahulu
    printf("Waktu pengurutan: %.2f detik\n", durationSec);
  }
}

// Fungsi untuk mengekstrak nama koleksi/dataset dari sebuah jalur file (path)
void extractCollectionName(const char *path, char *outName, size_t maxLen) {

  // Mencari karakter '/' terakhir dari kanan (format Linux/Mac)
  const char *fileName = strrchr(path, '/');

  // validasi  jika '/' tidak ketemu, cari karakter backslash '\' (format
  // Windows)
  if (!fileName)
    fileName = strrchr(path, '\\');

  // validasi jika pemisah direktori ditemukan, lewati karakter tersebut agar
  // menunjuk langsung ke nama file
  if (fileName)
    fileName++;

  // validasi
  else
    fileName = path; // jika tidak ada pemisah sama sekali, berarti input adalah
                     // nama filenya

  // sekarang `fileName` sudah berisi string nama file, contoh:
  // "docword.kos.txt"

  // memeriksa apakah 8 karakter pertama nama file adalah "docword."
  if (strncmp(fileName, "docword.", 8) == 0)
    fileName += 8; // sekarang `fileName` berubah menjadi contoh: "pubmed.txt"

  // memeriksa apakah awalan nama file adalah "vocab." (6 karakter)
  else if (strncmp(fileName, "vocab.", 6) == 0) {
    fileName += 6; // potong awalan, `fileName` menjadi "kos.txt"
  }

  // mengambil nama utama sebelum titik selanjutnya
  size_t i = 0;

  // loop akan berjalan selama karakter ada, karakter BUKAN titik '.', dan belum
  // melewati batas memori (maxLen - 1)
  while (fileName[i] && fileName[i] != '.' && i < maxLen - 1) {
    outName[i] = fileName[i]; // menyalin karakter nama file ke variabel output
    i++;
  }
  // menutup string hasil salinan dengan karakter null-terminator
  outName[i] = '\0';

  // periksa jika panjang karakter outName ternyata kosong (0) akibat format
  // file yang tidak sesuai
  if (strlen(outName) == 0) {
    // isi otomatis dengan nama default yaitu "dataset" agar program tidak error
    strncpy(outName, "dataset", maxLen - 1);
    outName[maxLen - 1] = '\0';
  }
}

void normalizePaths(const char *inputPath, char *docwordPath, char *vocabPath,
                    size_t maxLen) {
  char collection[128];

  extractCollectionName(inputPath, collection, sizeof(collection));

  // 2. Cari pemisah folder untuk direktori
  const char *lastSep = strrchr(inputPath, '/');
  if (!lastSep)
    lastSep = strrchr(inputPath, '\\');

  size_t dirLen = 0;
  if (lastSep)
    dirLen = lastSep - inputPath + 1;

  // 3. Bentuk ulang KEDUA path dengan ekstensi yang dijamin benar
  if (dirLen > 0) {
    snprintf(docwordPath, maxLen, "%.*sdocword.%s.txt", (int)dirLen, inputPath,
             collection);
    snprintf(vocabPath, maxLen, "%.*svocab.%s.txt", (int)dirLen, inputPath,
             collection);
  } else {
    snprintf(docwordPath, maxLen, "docword.%s.txt", collection);
    snprintf(vocabPath, maxLen, "vocab.%s.txt", collection);
  }
}

// Fungsi pembantu untuk membersihkan memori CachedResult
void freeCache(CachedResult *cache) {
  if (cache->isValid) {
    for (int i = 0; i < cache->count; i++) {
      free(cache->words[i]);
    }
    free(cache->words);
    free(cache->freqs);
    cache->isValid = 0;
  }
}

// Ubah fungsi agar menerima pointer dari CachedResult
int generateOutputFile(CachedResult *freqRes, CachedResult *rareRes,
                       const char *collection) {
  char fileName[256];

  // Tuliskan
  if (freqRes->isValid) {
    snprintf(fileName, sizeof(fileName), "top_frequent_%s.txt", collection);

    // Buka file dengan mode tulis
    FILE *fileFrequent = fopen(fileName, "w");

    if (fileFrequent != NULL) {

      fprintf(fileFrequent, "=================================== \n");
      fprintf(fileFrequent, "=== TOP %d FREQUENT %s DATA ===\n", freqRes->count,
              collection);
      fprintf(fileFrequent, "=================================== \n");

      // Tuliskan kata dan frekuensinya
      for (int i = 0; i < freqRes->count; i++)
        fprintf(fileFrequent, "%s (%lld)\n", freqRes->words[i],
                freqRes->freqs[i]);

      fprintf(fileFrequent, "-----------------------------------\n");
      fprintf(fileFrequent, "Waktu untuk mengurutkan: %.2f ms\n",
              freqRes->elapsed);

      fclose(fileFrequent);
      printf("File '%s' berhasil disimpan\n", fileName);
    } else {
      printf("Gagal membuka file '%s' untuk ditulis\n", fileName);
    }
  }

  // Tulis file Kata Terjarang
  if (rareRes->isValid) {
    snprintf(fileName, sizeof(fileName), "top_rare_%s.txt", collection);

    FILE *fileRare = fopen(fileName, "w");

    if (fileRare != NULL) {
      fprintf(fileRare, "=================================== \n");
      fprintf(fileRare, "=== TOP %d RARE %s DATA ===\n", rareRes->count,
              collection);
      fprintf(fileRare, "=================================== \n");

      for (int i = 0; i < rareRes->count; i++)
        fprintf(fileRare, "%s (%lld)\n", rareRes->words[i], rareRes->freqs[i]);

      fprintf(fileRare, "-----------------------------------\n");
      fprintf(fileRare, "Waktu untuk mengurutkan: %.2f ms\n", rareRes->elapsed);

      fclose(fileRare);
      printf("File '%s' berhasil disimpan\n", fileName);
    } else {
      printf("Gagal membuka file '%s' untuk ditulis\n", fileName);
    }
  }

  return 0;
}
