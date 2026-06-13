NAMA : FAIZ ASRA
NPM : 250810701100062
KELAS: B

================
DESKRIPSI PROYEK
================

Proyek ini adalah sistem pemrosesan teks intensif (Big Data) untuk menganalisis dataset berformat "Bag of Words" dari UCI Machine Learning Repository (seperti Enron, NIPS, KOS, NYTimes, dan PubMed). Program dirancang khusus untuk memproses file berukuran masif (hingga skala Gigabyte) secara efisien dan menentukan kombinasi kata tertentu menggunakan struktur data Heap.

Program menyediakan dua fungsionalitas analisis utama:

1. PENENTUAN KATA PALING SERING MUNCUL (TOP-K FREQUENT WORDS):
   Menggunakan pendekatan struktur data Min-Heap berkapasitas $k$ untuk menyaring dan mengisolasi sejumlah $k$ kata dengan jumlah frekuensi tertinggi dari seluruh dokumen.
2. PENENTUAN KATA PALING JARANG MUNCUL (TOP-K RARE WORDS):
   Menggunakan pendekatan struktur data Max-Heap berkapasitas $k$ untuk menyaring dan mengisolasi sejumlah $k$ kata dengan jumlah frekuensi terendah (namun tetap muncul atau > 0).

Program ini dilengkapi dengan fitur optimalisasi Fast I/O Streaming untuk membaca file besar, mekanisme Caching di memori RAM untuk mempercepat penampilan menu interaktif, serta pencatatan presisi waktu komputasi dalam satuan milidetik (ms) dan detik.

=============
STRUKTUR FILE
=============

Proyek ini diorganisasikan ke dalam modul-modul terpisah (modular approach) untuk memisahkan logika utama dengan manajemen heap dan fungsi pembantu:

1. Modul Utama (Root):
   -> Makefile : File otomatisasi kompilasi cross-platform yang mendukung deteksi otomatis OS Windows (GCC) dan Unix/macOS (Clang).

2. Modul Sumber (src/):
   -> src/main.c : Titik masuk utama (entry point) program, mengelola alur menu interaktif, parsing data triplet, dan koordinasi cache.
   -> src/heap.h : Header file berisi definisi struktur data `HeapNode`, `MinHeap`, `MaxHeap`, dan prototipe operasi heap.
   -> src/heap.c : Implementasi operasi heap meliputi inisialisasi, penyisipan (insert), heapify down, pengurutan (heapsort), dan pembalikan urutan array.
   -> src/utils.h : Header file berisi prototipe fungsi manajemen string, ekstraksi nama koleksi, serta manipulasi cache.
   -> src/utils.c : Implementasi normalisasi path file, ekstraksi nama dataset, pelepasan memori cache (`freeCache`), dan penulisan berkas output.

=================
KONSTANTA PROGRAM
=================

FAST_BUFFER_SIZE = 8.388.608 Byte (8 MB)
Ukuran alokasi buffer blok memori internal yang digunakan untuk membaca file kontainer `docword.*.txt` secara sekuensial (streaming block) untuk memangkas overhead pemanggilan I/O harddisk.

RENTANG NILAI K = 10 < k < 150
Batasan validasi jumlah elemen Top-K kata teratas maupun terbawah yang dapat diminta oleh pengguna di dalam menu interaktif.

===============================
FAST STREAMING & INLINE PARSING
===============================

Untuk menangani ukuran file dataset yang sangat besar (seperti PubMed atau NYTimes), program tidak membaca file baris-per-baris menggunakan fungsi standar `fgets` lambat. Program menerapkan strategi:

1. Membaca file dalam potongan blok memori besar (8 MB) sekaligus ke dalam RAM menggunakan `fread`.
2. Memproses pecahan baris terpotong secara aman menggunakan penanda pointer `leftOvers` dan fungsi penataan memori `memmove`.
3. Melakukan inline parsing string biner manual berkecepatan tinggi tanpa fungsi `sscanf`. Karakter spasi/tab dilompati, `docID` diabaikan, sedangkan `wordID` dan `count` dikonversi langsung menjadi bilangan bulat secara matematis berbasis kode ASCII (`wordID * 10 + (*p - '0')`). Frekuensi langsung diakumulasikan ke dalam array `freq[wordID]` secara real-time.

=========================
STRUKTUR DATA & HEAP SORT
=========================

Program memanfaatkan struktur pohon biner lengkap (complete binary tree) yang diimplementasikan secara efisien di dalam array flat. Operasi pemeliharaan heap (_heapify_) dilakukan secara **iteratif** untuk menjamin efisiensi memori tingkat tinggi dan menghindari bahaya Stack Overflow.

## 1. MENU 1 – MIN-HEAP (Top-K Frequent Words)

- **Logika:**
  -> Digunakan untuk mencari kata yang paling sering muncul.

- **Prinsip Kerja:**
  -> Array min-heap berkapasitas maksimum 'k' diisi. Saat elemen ke-(k+1) hingga akhir diperiksa, jika frekuensinya lebih besar dibanding root (elemen terkecil di heap saat itu), maka root didepak dan digantikan elemen baru melalui `minHeapifyDown`. Setelah seluruh kosakata selesai dipindai, dilakukan `minHeapSort` untuk menyusun hasil akhir secara terurut menurun (descending).

## 2. MENU 2 – MAX-HEAP (Top-K Rare Words)

- **Logika:**
  -> Digunakan untuk mencari kata yang paling jarang muncul (dengan frekuensi > 0).

- **Prinsip Kerja:**
  -> Menggunakan max-heap berkapasitas maksimum 'k'. Jika elemen kosakata baru memiliki frekuensi lebih kecil daripada nilai root max-heap (elemen terbesar di dalam heap saat itu), elemen root dikeluarkan dan diganti dengan elemen baru. Setelah selesai, dijalankan `maxHeapSort` dan `maxHeapReverse` sehingga urutannya rapi dari kata yang frekuensinya paling sedikit ke yang lebih tinggi.

=======================
MEKANISME CACHING (RAM)
=======================

Program menggunakan `struct CachedResult` untuk mengimplementasikan memori jangka pendek (_cache_) internal di RAM:

```c
typedef struct {
    char **words;     // Array teks kata hasil pengurutan
    long long *freqs; // Array frekuensi paralel dari kata tersebut
    int count;        // Jumlah elemen aktif di dalam cache
    int k;            // Nilai k yang digunakan pada pencarian terakhir
    double elapsed;   // Waktu eksekusi heapsort (ms)
    int isValid;      // Flag validitas data (1 = siap, 0 = kosong)
} CachedResult;
```

Ketika pengguna mengeksekusi pengurutan di Menu 1 atau Menu 2, hasilnya dibekukan ke dalam variabel 'freqCache' dan 'rareCache'. Ketika pengguna memilih Menu 3 atau Menu 4, data langsung dibaca dari objek cache ini secara instan ($O(1)$) tanpa perlu memproses ulang data mentah file teks dari awal.

==============
SIMPAN KE FILE
==============

Setiap kali proses kalkulasi di Menu 1 atau Menu 2 selesai dilakukan, program secara otomatis mengeksekusi prosedur internal untuk membuat dan menyimpan seluruh hasil laporan pengurutan ke file fisik eksternal baru. Nama file luaran disesuaikan secara otomatis berdasarkan nama dataset aktif yang dianalisis:

**_ Format Penamaan:_** top*frequent*[nama_koleksi].txt dan top*rare*[nama_koleksi].

txtBerkas memuat informasi header berupa identitas koleksi data, nilai batasan k, durasi performa kecepatan pengurutan, serta visualisasi daftar kata yang bersanding dengan angka frekuensi kemunculannya.

===============
TAMPILKAN WAKTU
===============

Pencatatan durasi waktu performa sistem menggunakan pustaka makro clock(). Program menerapkan cetakan cerdas kondisional:

-> Jika durasi waktu pemrosesan berada di bawah angka 1000.0 milidetik, durasi waktu akan dipajang langsung dalam satuan Milidetik (ms).-
-> Jika durasi waktu menyentuh atau melampaui rentang 1000.0 milidetik, visualisasi konversi otomatis disajikan dalam format satuan Detik.

================
MANAJEMEN MEMORI
================

Seluruh komponen alokasi memori dinamis dikendalikan secara mandiri dan ketat untuk menjamin program bebas dari kebocoran memori (memory leaks):

**_ malloc() _**
->Digunakan untuk alokasi awal array penampung string kosakata vocab, buffer streaming besar (8 MB), serta memori dinamis penampung node heap.
**_ calloc() _**
->Digunakan untuk mengalokasikan array akumulator frekuensi berkapasitas W + 1 dan memastikan seluruh kondisi awalnya bersih bernilai nol.
**_ free() _**
->Prosedur pembebasan memori diterapkan menyeluruh pada setiap elemen string, array pointer, memori buffer, dan struktur cache.

Penanganan pengosongan objek cache dilakukan via fungsi khusus freeCache() sebelum ruang memori dialokasikan ulang untuk pengujian baru.
