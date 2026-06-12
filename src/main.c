#include "heap.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// Ukuran buffer untuk pembacaan data langsung dari file (8 MB)
#define FAST_BUFFER_SIZE (8 * 1024 * 1024)

int main(void) {
  FILE *fp = NULL;
  FILE *vfp = NULL;
  char docwordPath[512];
  char vocabPath[512];
  char collection[128];
  char userInput[512];

  // Inisialisasi cache memori untuk kata paling sering dan paling jarang dalam
  // file
  CachedResult freqCache = {.isValid = 0};
  CachedResult rareCache = {.isValid = 0};

  // Meminta input file sampai mendapatkan file yang valid
  // Meminta input file sampai mendapatkan file yang valid
  while (1) {
    printf("\n================================================================="
           "==\n");
    printf("Masukkan file yang ingin anda urutkan: ");

    if (fgets(userInput, sizeof(userInput), stdin) == NULL) {
      printf("\n==============================================================="
             "====\n");
      continue;
    }

    // Hapus karakter newline (\r dan \n) di akhir input
    userInput[strcspn(userInput, "\r\n")] = '\0';

    if (strlen(userInput) == 0)
      continue;

    // Lakukan normalisasi JALUR terlebih dahulu sebelum membuka
    // file apa pun!
    normalizePaths(userInput, docwordPath, vocabPath, sizeof(docwordPath));

    // Ambil nama koleksi dataset dari hasil path yang sudah dinormalisasi
    extractCollectionName(docwordPath, collection, sizeof(collection));

    // Buka file docword yang jalurnya sudah dijamin benar
    fp = fopen(docwordPath, "r");
    if (!fp) {
      printf("\n==============================================================="
             "====\n");
      printf("File data '%s' tidak ditemukan, silakan coba lagi\n",
             docwordPath);
      printf("================================================================="
             "==\n");
      continue;
    }

    // Buka file vocab yang jalurnya juga sudah dijamin benar
    vfp = fopen(vocabPath, "r");
    if (!vfp) {
      printf("\n==============================================================="
             "====\n");
      printf("File vocab '%s' tidak ditemukan, silakan coba lagi\n", vocabPath);
      printf("================================================================="
             "==\n");
      fclose(fp); // Tutup fp docword jika vocab-nya tidak ada
      fp = NULL;
      continue;
    }
    break;
  }

  // Membaca header baris 1-3 dari docword: D (Dokumen), W (Vocabulary), N(Total
  // entry)
  char line[128];
  int D = 0, W = 0;
  long long N = 0;

  if (!fgets(line, sizeof(line), fp)) {
    printf("\n==============================================================="
           "====\n");
    printf("File docword kosong. Silahkan pilih file yang lain");
    printf("\n==============================================================="
           "====\n");

    // Menutup file
    fclose(fp);
    fclose(vfp);
    return 1;
  }

  // Mengambil nilai D dari baris pertama
  D = atoi(line);

  if (!fgets(line, sizeof(line), fp)) {
    printf("\n=================================\n");
    printf("Gagal membaca W dari file docword");
    printf("\n=================================\n");

    // tutup
    fclose(fp);
    fclose(vfp);
    return 1;
  }

  // Mengambil nilai W dari baris kedua
  W = atoi(line);

  if (!fgets(line, sizeof(line), fp)) {
    printf("\n=================================\n");
    printf("Gagal membaca N dari file docword");
    printf("\n=================================\n");

    // tutup
    fclose(fp);
    fclose(vfp);
    return 1;
  }

  // Mengambil nilai N dari baris ketiga
  N = atoll(line);

  printf("\n==============================================================="
         "====\n");
  printf("Dataset Info: \nD = %d\nW = %d\nN = %lld", D, W, N);
  printf("\n==============================================================="
         "====\n");

  // Alokasi memori array vocabulary dengan ukuran W + 1 (1-indexed)
  char **vocab = malloc((W + 1) * sizeof(char *));

  if (!vocab) {
    printf("\n==============================================================="
           "====\n");
    printf("Gagal mengalokasikan memori untuk vocabulary");
    printf("\n==============================================================="
           "====\n");

    // tutup
    fclose(fp);
    fclose(vfp);
    return 1;
  }

  // Muat kata-kata dari file vocabulary baris demi baris ke dalam array vocab
  for (int i = 1; i <= W; i++) {

    // alokasi buffer untuk menampung kata-kata
    char word[256];

    if (fgets(word, sizeof(word), vfp)) {
      word[strcspn(word, "\r\n")] = '\0';
      vocab[i] = strdup(word);
    } else
      vocab[i] = strdup("");
  }
  fclose(vfp);

  // Alokasi array frekuensi kata dengan ukuran W + 1 diinisialisasi dengan 0
  long long *freq = calloc(W + 1, sizeof(long long));
  if (!freq) {
    printf("\n==============================================================="
           "====\n");
    printf("Gagal mengalokasikan memori untuk array frekuensi");
    printf("\n==============================================================="
           "====\n");

    // membebaskan memori yang udah dialokasikan
    for (int i = 1; i <= W; i++)
      free(vocab[i]);

    free(vocab);
    fclose(fp);
    return 1;
  }

  // Membaca docword secara streaming menggunakan block reader berukuran 8 MB
  printf("\n==============================================================="
         "====\n");
  printf("Membaca data docword secara streaming dan mengumpulkan frekuensi...");
  printf("\n==============================================================="
         "====\n");

  // Mulai menghitung waktu pembacaan
  clock_t readStart = clock();

  // Alokasikan buffer
  char *buffer = malloc(FAST_BUFFER_SIZE + 1);

  // validasi berhasil atau tidaknya alokasi buffer
  if (!buffer) {
    printf("\n=====================================\n");
    printf("Gagal mengalokasikan buffer pembacaan");
    printf("\n=====================================\n");

    // Bebaskan memori yang udah dialokasikan
    for (int i = 1; i <= W; i++)
      free(vocab[i]);

    // Bebaskan memori dan tutup file
    free(vocab);
    free(freq);
    fclose(fp);
    return 1;
  }

  // Variabel untuk menyimpan sisa data antar potongan (jika ada baris tidak
  // lengkap)
  size_t leftOvers = 0;

  // Variabel untuk menyimpan jumlah byte yang dibaca dari file dalam setiap
  // potongan
  size_t bytesRead = 0;

  // Loop membaca file dalam potongan besar (FAST_BUFFER_SIZE)
  while ((bytesRead = fread(buffer + leftOvers, 1, FAST_BUFFER_SIZE - leftOvers,
                            fp)) > 0) {
    // Gabungkan sisa potongan sebelumnya dengan data baru
    size_t totalLen = bytesRead + leftOvers;
    buffer[totalLen] = '\0';

    // pointer yang menunjuk ke awal potongan data
    char *ptr = buffer;

    // pointer yang menunjuk ke akhir potongan data
    char *end = buffer + totalLen;

    // Proses semua baris lengkap yang ditemukan dalam buffer saat ini
    while (ptr < end) {
      // Mencari karakter newline
      char *nl = strchr(ptr, '\n');

      // Jika tidak ada newline, hentikan
      if (!nl)
        break;

      // ubah newline menjadi null-terminator untuk memproses baris
      *nl = '\0';

      // Parsing baris "docID wordID count" secara manual dan cepat (inline)
      char *p = ptr;

      // Lewati spasi dan tab di awal baris
      while (*p && (*p == ' ' || *p == '\t'))
        p++;

      if (*p) {
        // Lewati nilai docID karena tidak digunakan
        while (*p && *p >= '0' && *p <= '9')
          p++;

        // Lewati spasi dan tab setelah docID
        while (*p && (*p == ' ' || *p == '\t'))
          p++;

        // Parsing wordID
        int wordID = 0;

        // Parsing wordID dari string
        while (*p && *p >= '0' && *p <= '9') {
          wordID = wordID * 10 + (*p - '0');
          p++;
        }
        // Lewati spasi dan tab setelah wordID
        while (*p && (*p == ' ' || *p == '\t'))
          p++;

        // Parsing count
        int count = 0;

        // Parsing count dari string
        while (*p && *p >= '0' && *p <= '9') {
          count = count * 10 + (*p - '0');
          p++;
        }

        // Akumulasikan frekuensi kemunculan kata jika wordID valid
        if (wordID >= 1 && wordID <= W) {
          freq[wordID] += count;
        }
      }

      // Pindahkan pointer ke awal baris berikutnya
      ptr = nl + 1;
    }

    // Pindahkan sisa potongan baris terakhir yang terpotong ke awal buffer
    if (ptr < end) {
      leftOvers = end - ptr;
      memmove(buffer, ptr, leftOvers);
    }

    // Jika tidak ada sisa baris, reset leftOvers
    else
      leftOvers = 0;
  }

  // Proses baris terakhir yang tersisa di akhir file jika tidak diakhiri
  // newline
  if (leftOvers > 0) {
    buffer[leftOvers] = '\0';
    char *p = buffer;
    while (*p && (*p == ' ' || *p == '\t'))
      p++;
    if (*p) {
      // Lewati docID
      while (*p && *p >= '0' && *p <= '9') {
        p++;
      }

      // Lewati spasi dan tab setelah docID
      while (*p && (*p == ' ' || *p == '\t'))
        p++;

      int wordID = 0;

      // Parsing wordID dari string
      while (*p && *p >= '0' && *p <= '9') {
        wordID = wordID * 10 + (*p - '0');
        p++;
      }

      // Lewati spasi dan tab setelah wordID
      while (*p && (*p == ' ' || *p == '\t'))
        p++;

      int count = 0;

      // Parsing count dari string
      while (*p && *p >= '0' && *p <= '9') {
        count = count * 10 + (*p - '0');
        p++;
      }

      // Akumulasikan frekuensi kemunculan kata jika wordID valid
      if (wordID >= 1 && wordID <= W) {
        freq[wordID] += count;
      }
    }
  }

  // Tutup file dan bebaskan memori buffer
  fclose(fp);
  free(buffer);

  // Hitung waktu pembacaan file
  clock_t readEnd = clock();
  double readElapsedMs =
      ((double)(readEnd - readStart) / CLOCKS_PER_SEC) * 1000.0;

  // Tampilkan waktu pembacaan file (logika cetak cerdas)
  if (readElapsedMs < 1000.0) {
    printf("\n==============================================================="
           "====\n");
    printf("Selesai membaca file. Waktu: %.2f ms", readElapsedMs);
    printf("\n==============================================================="
           "====\n");
  } else {
    printf("\n==============================================================="
           "====\n");
    printf("Selesai membaca file. Waktu: %.2f detik", readElapsedMs / 1000.0);
    printf("\n==============================================================="
           "====\n");
  }

  // Loop menu pilihan interaktif
  int choice = 0;
  while (1) {
    printf("\n/*************************** Petunjuk "
           "****************************/\n");
    printf("Langkah-langkah yang harus dilakukan selanjutnya adalah:\n");
    printf("1. Buka file docword dan vocabulary.\n");
    printf("2. Baca data baris per baris dan tempatkan data dalam struktur "
           "data\n");
    printf("   yang diperlukan dan jumlahkan frekuensi kata secara benar.\n");
    printf("3. Selanjutnya, MENU pilihan ditampilkan dan pilihan ditentukan "
           "oleh\n   user. Proses dilakukan berdasarkan pilihan user.\n");
    printf("4. Pilih menu 1 dan 2 terlebih dahulu agar pengurutan "
           "tersimpan\n   kedalam file baru\n");
    printf("/************************** Akhir Petunjuk "
           "***********************/\n\n");
    printf("\n==============================================================="
           "====\n");
    printf("                             Pilihan");
    printf("\n==============================================================="
           "====\n");
    printf("\n1) Tentukan kata paling SERING muncul (min-heap)\n");
    printf("2) Tentukan kata paling JARANG muncul (max-heap)\n");
    printf("3) Tampilkan kata paling SERING muncul\n");
    printf("4) Tampilkan kata paling JARANG muncul\n");
    printf("5) Selesai\n\n");
    printf("Pilihan anda: ");

    char choiceStr[64];

    // input pilihan user
    if (fgets(choiceStr, sizeof(choiceStr), stdin) == NULL)
      continue;

    // Ubah string menjadi integer
    choice = atoi(choiceStr);
    printf("==============================================================="
           "====\n");

    // Pilihan No 1
    if (choice == 1) {
      int k = 0;

      // Loop validasi input nilai k
      while (1) {
        printf("\n--------------------------------------\n");
        printf("Tentukan nilai k (10 < k < 150): ");

        // input nilai k
        char k_str[64];

        // input nilai k
        if (fgets(k_str, sizeof(k_str), stdin) == NULL)
          continue;
        printf("--------------------------------------\n");

        // Ubah string menjadi integer
        k = atoi(k_str);

        // Validasi nilai k
        if (k > 10 && k < 150)
          break;

        // Pesan error jika nilai k tidak valid
        printf(
            "\n==============================================================="
            "====\n");
        printf("Nilai k tidak valid. Harus berada di rentang 10 < k < 150");
        printf(
            "\n==============================================================="
            "====\n");
      }

      // Alokasi memori dinamis untuk buffer elemen min-heap berukuran k
      HeapNodePtr topFrequent = malloc(k * sizeof(HeapNode));
      if (!topFrequent) {
        printf(
            "\n==============================================================="
            "====\n");
        printf("Gagal mengalokasikan memori untuk heap");
        printf(
            "\n==============================================================="
            "====\n");
        continue;
      }

      // Inisialisasi min-heap
      MinHeap heap;
      minHeapInit(&heap, topFrequent, k);

      // Hitung waktu mulai
      clock_t startTime = clock();

      // Masukkan setiap kata ke dalam min-heap
      for (int i = 1; i <= W; i++) {

        // Masukkan data node
        HeapNode node = {.freq = freq[i], .wordID = i};
        minHeapInsert(&heap, node);
      }
      // Urutkan min-heap menggunakan heapsort (in-place descending)
      minHeapSort(&heap);

      // Hitung waktu selesai
      clock_t endTime = clock();
      double elapsed =
          ((double)(endTime - startTime) / CLOCKS_PER_SEC) * 1000.0;

      // Simpan salinan hasil ke dalam potongan memori
      freeCache(&freqCache);
      freqCache.words = malloc(heap.size * sizeof(char *));
      freqCache.freqs = malloc(heap.size * sizeof(long long));
      freqCache.count = heap.size;
      freqCache.k = k;
      freqCache.elapsed = elapsed;

      // Menyalin data dari heap ke potongan memori
      for (int i = 0; i < heap.size; i++) {
        freqCache.words[i] = strdup(vocab[heap.data[i].wordID]);
        freqCache.freqs[i] = heap.data[i].freq;
      }

      // Menandai potongan sebagai valid
      freqCache.isValid = 1;

      // Panggil generate otomatis setelah menu 1 dieksekusi agar file
      // langsung terbentuk
      generateOutputFile(&freqCache, &rareCache, collection);

      // Bebaskan memori
      free(topFrequent);
    }

    // Pilihan No 2
    else if (choice == 2) {
      int k = 0;

      // Validasi input nilai k
      while (1) {
        printf("\n--------------------------------------\n");
        printf("Tentukan nilai k (10 < k < 150): ");

        // Input nilai k
        char k_str[64];
        if (fgets(k_str, sizeof(k_str), stdin) == NULL)
          continue;
        printf("--------------------------------------\n");

        // Ubah string menjadi integer
        k = atoi(k_str);

        // Validasi nilai k
        if (k > 10 && k < 150)
          break;

        // Pesan error jika nilai k tidak valid
        printf(
            "\n==============================================================="
            "====\n");
        printf("Nilai k tidak valid. Harus berada di rentang 10 < k <150");
        printf(
            "\n==============================================================="
            "====\n");
      }

      // Alokasi memori dinamis untuk buffer elemen max-heap berukuran k
      HeapNodePtr topRare = malloc(k * sizeof(HeapNode));
      if (!topRare) {
        printf(
            "\n==============================================================="
            "====\n");
        printf("Gagal mengalokasikan memori untuk heap");
        printf(
            "\n==============================================================="
            "====\n");
        continue;
      }

      // Inisialisasi max-heap
      MaxHeap heap;
      maxHeapInit(&heap, topRare, k);

      // Hitung waktu mulai
      clock_t startTime = clock();

      // Masukkan setiap kata ke dalam max-heap
      for (int i = 1; i <= W; i++) {
        // Abaikan kata yang memiliki frekuensi 0
        if (freq[i] > 0) {
          HeapNode node = {.freq = freq[i], .wordID = i};
          maxHeapInsert(&heap, node);
        }
      }
      // Jalankan heapsort dan balikkan urutannya menjadi descending (frekuensi
      // terkecil di bawah)
      maxHeapSort(&heap);
      maxHeapReverse(&heap);

      // Hitung waktu selesai
      clock_t endTime = clock();
      double elapsed =
          ((double)(endTime - startTime) / CLOCKS_PER_SEC) * 1000.0;

      // Simpan salinan hasil kata paling jarang ke cache memori
      freeCache(&rareCache);
      rareCache.words = malloc(heap.size * sizeof(char *));
      rareCache.freqs = malloc(heap.size * sizeof(long long));
      rareCache.count = heap.size;
      rareCache.k = k;
      rareCache.elapsed = elapsed;

      // Menyalin data dari heap ke potongan memori
      for (int i = 0; i < heap.size; i++) {
        rareCache.words[i] = strdup(vocab[heap.data[i].wordID]);
        rareCache.freqs[i] = heap.data[i].freq;
      }

      // Menandai potongan sebagai valid
      rareCache.isValid = 1;

      // Simpan hasil pencarian Top-K Rare ke file eksternal dan bebaskan memori
      generateOutputFile(&freqCache, &rareCache, collection);
      free(topRare);
    }

    // Pilihan No 3
    else if (choice == 3) {
      // Menampilkan hasil kata tersering langsung dari memori
      if (freqCache.isValid) {
        printf("\n--- %d Kata Paling Sering (%s) ---\n", freqCache.k,
               collection);

        // Menampilkan kata paling sering muncul
        for (int i = 0; i < freqCache.count; i++)
          printf("%s (%lld)\n", freqCache.words[i], freqCache.freqs[i]);

        // Menampilkan waktu yang dibutuhkan untuk heapsort
        if (freqCache.elapsed < 1000.0) {
          printf("-----------------------------------------\n");
          printf("Waktu untuk mengurutkan: %.2f ms\n", freqCache.elapsed);
        } else {
          printf("-----------------------------------------\n");
          printf("Waktu untuk mengurutkan: %.2f detik\n",
                 freqCache.elapsed / 1000.0);
        }
      }

      // Jika data belum ada di memori
      else {
        printf(
            "\n==============================================================="
            "====\n");
        printf("Data hasil kata paling sering belum ada di memori Silakan"
               "pilih menu 1 terlebih dahulu\n");
        printf(
            "\n==============================================================="
            "====\n");
      }
    }

    else if (choice == 4) {
      // Menampilkan hasil kata terjarang langsung dari memori(tanpa baca ulang
      // file)
      if (rareCache.isValid) {
        printf("\n--- %d Kata Paling Jarang (%s) ---\n", rareCache.k,
               collection);
        for (int i = 0; i < rareCache.count; i++)
          printf("%s (%lld)\n", rareCache.words[i], rareCache.freqs[i]);

        if (rareCache.elapsed < 1000.0) {
          printf("-----------------------------------------\n");
          printf("Waktu untuk mengurutkan: %.2f ms\n", rareCache.elapsed);
        } else {
          printf("-----------------------------------------\n");
          printf("Waktu untuk mengurutkan: %.2f detik\n",
                 rareCache.elapsed / 1000.0);
        }
      }

      else {
        printf(
            "\n==============================================================="
            "====\n");
        printf("Data hasil kata paling jarang belum ada di memori Silakan"
               "pilih menu 2 terlebih dahulu");
        printf(
            "\n==============================================================="
            "====\n");
      }
    }

    else if (choice == 5) {
      // Bebaskan semua sisa alokasi memori saat keluar
      printf("\n==============================================================="
             "====\n");
      printf("Membebaskan memori dan keluar...");

      // Bebaskan memori yang dialokasikan untuk vocab
      for (int i = 1; i <= W; i++) {
        free(vocab[i]);
      }

      // Bebaskan memori yang dialokasikan untuk vocab
      free(vocab);

      // Bebaskan memori yang dialokasikan untuk freq
      free(freq);

      // Bebaskan memori yang dialokasikan untuk freqCache
      freeCache(&freqCache);

      // Bebaskan memori yang dialokasikan untuk rareCache
      freeCache(&rareCache);

      // Menampilkan pesan selesai
      printf("Selesai dan Terima kasih");
      printf("\n==============================================================="
             "====\n\n");
      break;

    } else {
      printf("\n==============================================================="
             "====\n");
      printf("Pilihan tidak valid. Silakan masukkan 1-5");
      printf("\n==============================================================="
             "====\n");
    }
  }

  return 0;
}
