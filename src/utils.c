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

void getVocabPath(const char *docwordPath, char *vocabPath, size_t maxLen) {

  // cari karakter pemisah folder terakhir untuk menentukan direktori kerja
  const char *lastSep = strrchr(docwordPath, '/');
  if (!lastSep)
    lastSep = strrchr(docwordPath, '\\');

  size_t dirLen = 0;
  // termasuk karakter pemisah (/)
  if (lastSep)
    dirLen = lastSep - docwordPath + 1;

  char collection[128];
  extractCollectionName(docwordPath, collection, sizeof(collection));

  // gabungkan direktori dengan nama vocabulary yang sesuai
  if (dirLen > 0) {
    snprintf(vocabPath, maxLen, "%.*svocab.%s.txt", (int)dirLen, docwordPath,
             collection);
  } else {
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

  // 1. Tulis file Kata Tersering
  if (freqRes->isValid) {
    snprintf(fileName, sizeof(fileName), "top_frequent_%s.txt", collection);

    FILE *fileFrequent = fopen(fileName, "w");

    if (fileFrequent != NULL) {
      fprintf(fileFrequent, "=== TOP %d KATA PALING SERING (%s) ===\n",
              freqRes->k, collection);
      fprintf(fileFrequent, "%-5s %-30s %s\n", "No.", "Kata", "Frekuensi");
      fprintf(fileFrequent, "----------------------------------------------\n");

      for (int i = 0; i < freqRes->count; i++)
        fprintf(fileFrequent, "%-5d %-30s %lld kali\n", i + 1,
                freqRes->words[i], freqRes->freqs[i]);

      // Tulis waktu pengurutan ke file
      fprintf(fileFrequent, "----------------------------------------------\n");
      if (freqRes->elapsed < 1000.0)
        fprintf(fileFrequent, "Waktu pengurutan: %.2f ms\n", freqRes->elapsed);
      else
        fprintf(fileFrequent, "Waktu pengurutan: %.2f detik\n",
                freqRes->elapsed / 1000.0);

      fclose(fileFrequent);
      printf("File '%s' berhasil disimpan.\n", fileName);
    } else {
      printf("Gagal membuka file '%s' untuk ditulis.\n", fileName);
    }
  }

  // 2. Tulis file Kata Terjarang
  if (rareRes->isValid) {
    snprintf(fileName, sizeof(fileName), "top_rare_%s.txt", collection);
    FILE *fileRare = fopen(fileName, "w");
    if (fileRare != NULL) {
      fprintf(fileRare, "=== TOP %d KATA PALING JARANG (%s) ===\n", rareRes->k,
              collection);
      fprintf(fileRare, "%-5s %-30s %s\n", "No.", "Kata", "Frekuensi");
      fprintf(fileRare, "----------------------------------------------\n");
      for (int i = 0; i < rareRes->count; i++) {
        fprintf(fileRare, "%-5d %-30s %lld kali\n", i + 1, rareRes->words[i],
                rareRes->freqs[i]);
      }
      // Tulis waktu pengurutan ke file
      fprintf(fileRare, "----------------------------------------------\n");
      if (rareRes->elapsed < 1000.0)
        fprintf(fileRare, "Waktu pengurutan: %.2f ms\n", rareRes->elapsed);
      else
        fprintf(fileRare, "Waktu pengurutan: %.2f detik\n",
                rareRes->elapsed / 1000.0);

      fclose(fileRare);
      printf("File '%s' berhasil disimpan.\n", fileName);
    } else {
      printf("Gagal membuka file '%s' untuk ditulis.\n", fileName);
    }
  }

  return 0;
}
