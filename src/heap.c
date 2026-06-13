#include "heap.h"
#include <stddef.h>

// Inisialisasi MinHeap: Set pointer buffer data, ukuran awal 0, dan kapasitas maksimal
void minHeapInit(MinHeap *heap, HeapNodePtr data_buffer, int capacity)
{
  heap->data = data_buffer;
  heap->size = 0;
  heap->capacity = capacity;
}

// Menjaga properti Min-Heap ke arah bawah dari indeks tertentu
void minHeapifyDown(MinHeap *heap, int idx)
{
  int smallest = idx;
  int left = 2 * idx + 1;  // Anak kiri
  int right = 2 * idx + 2; // Anak kanan

  // Jika anak kiri lebih kecil dari node saat ini
  if (left < heap->size && heap->data[left].freq < heap->data[smallest].freq)
    smallest = left;

  // Jika anak kanan lebih kecil dari node terkecil sementara
  if (right < heap->size && heap->data[right].freq < heap->data[smallest].freq)
    smallest = right;

  // Jika posisi terkecil berubah, lakukan penukaran dan panggil secara rekursif
  if (smallest != idx)
  {
    HeapNode temp = heap->data[idx];
    heap->data[idx] = heap->data[smallest];
    heap->data[smallest] = temp;
    minHeapifyDown(heap, smallest);
  }
}

// Memasukkan node baru ke dalam Min-Heap
void minHeapInsert(MinHeap *heap, HeapNode node)
{
  // Jika heap belum penuh, tambahkan elemen di bagian paling akhir dan jalankan
  // heapify up
  if (heap->size < heap->capacity)
  {
    heap->data[heap->size] = node;
    int current = heap->size;
    heap->size++;

    // Geser elemen baru ke atas selama nilainya lebih kecil dari parent-nya
    while (current > 0)
    {
      int parent = (current - 1) / 2;

      if (heap->data[current].freq < heap->data[parent].freq)
      {
        HeapNode temp = heap->data[current];
        heap->data[current] = heap->data[parent];
        heap->data[parent] = temp;
        current = parent;
      }
      else
        break;
    }
  }
  // Jika heap penuh, bandingkan dengan root (nilai minimum dalam heap)
  else
  {
    // Jika frekuensi elemen baru lebih besar dari root, buang root dan
    // tempatkan elemen baru
    if (node.freq > heap->data[0].freq)
    {
      heap->data[0] = node;
      minHeapifyDown(heap, 0); // Atur ulang properti heap ke arah bawah
    }
  }
}

// Mengurutkan Min-Heap secara in-place (descending)
void minHeapSort(MinHeap *heap)
{
  int original_size = heap->size;

  while (heap->size > 1)
  {
    // Tukar root (nilai minimum) dengan elemen paling akhir pada heap aktif
    HeapNode temp = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->data[heap->size - 1] = temp;

    // Kurangi ukuran heap aktif (elemen terkecil di akhir array diisolasi)
    heap->size--;

    // Jalankan heapify pada root untuk merapikan sisa heap aktif
    minHeapifyDown(heap, 0);
  }
  // Pulihkan ukuran heap semula agar data array dapat diakses penuh
  heap->size = original_size;
}

// Inisialisasi MaxHeap: Set pointer buffer data, ukuran awal 0, dan kapasitas maksimal
void maxHeapInit(MaxHeap *heap, HeapNodePtr data_buffer, int capacity)
{
  heap->data = data_buffer;
  heap->size = 0;
  heap->capacity = capacity;
}

// Menjaga properti Max-Heap ke arah bawah dari indeks tertentu
void maxHeapifyDown(MaxHeap *heap, int idx)
{
  int largest = idx;
  int left = 2 * idx + 1;  // Anak kiri
  int right = 2 * idx + 2; // Anak kanan

  // Jika anak kiri lebih besar dari node saat ini
  if (left < heap->size && heap->data[left].freq > heap->data[largest].freq)
    largest = left;

  // Jika anak kanan lebih besar dari node terbesar sementara
  if (right < heap->size && heap->data[right].freq > heap->data[largest].freq)
    largest = right;

  // Jika posisi terbesar berubah, lakukan penukaran dan panggil secara rekursif
  if (largest != idx)
  {
    HeapNode temp = heap->data[idx];
    heap->data[idx] = heap->data[largest];
    heap->data[largest] = temp;
    maxHeapifyDown(heap, largest);
  }
}

// Memasukkan node baru ke dalam Max-Heap
void maxHeapInsert(MaxHeap *heap, HeapNode node)
{
  // Jika heap belum penuh, tambahkan elemen di bagian paling akhir dan jalankan heapify up
  if (heap->size < heap->capacity)
  {
    heap->data[heap->size] = node;
    int current = heap->size;
    heap->size++;

    // Geser elemen baru ke atas selama nilainya lebih besar dari parent-nya
    while (current > 0)
    {
      int parent = (current - 1) / 2;

      if (heap->data[current].freq > heap->data[parent].freq)
      {
        HeapNode temp = heap->data[current];
        heap->data[current] = heap->data[parent];
        heap->data[parent] = temp;
        current = parent;
      }
      else
        break;
    }
  }
  // Jika heap penuh, bandingkan dengan root (nilai maksimum dalam heap)
  else
  {
    if (node.freq < heap->data[0].freq)
    {
      heap->data[0] = node;
      maxHeapifyDown(heap, 0); // Atur ulang properti heap ke arah bawah
    }
  }
}

// Mengurutkan Max-Heap secara in-place (menghasilkan urutan ascending/menaik)
void maxHeapSort(MaxHeap *heap)
{
  int original_size = heap->size;

  while (heap->size > 1)
  {
    // Tukar root (nilai maksimum) dengan elemen paling akhir pada heap aktif
    HeapNode temp = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->data[heap->size - 1] = temp;

    // Kurangi ukuran heap aktif (elemen terbesar di akhir array diisolasi)
    heap->size--;

    // Jalankan heapify pada root untuk merapikan sisa heap aktif
    maxHeapifyDown(heap, 0);
  }
  // Pulihkan ukuran heap semula agar data array dapat diakses penuh
  heap->size = original_size;
}

// Balikkan urutan array hasil Max-Heap untuk mendapatkan urutan descending
void maxHeapReverse(MaxHeap *heap)
{
  int l = 0, r = heap->size - 1;

  while (l < r)
  {
    HeapNode temp = heap->data[l];
    heap->data[l] = heap->data[r];
    heap->data[r] = temp;
    l++;
    r--;
  }
}
