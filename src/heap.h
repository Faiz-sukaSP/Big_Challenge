#ifndef HEAP_H
#define HEAP_H

// Alias
typedef struct HeapNode *HeapNodePtr;

// Struktur data untuk menampung satu node/elemen kosakata
typedef struct HeapNode
{
  long long freq;
  int wordID;
} HeapNode;

// Struktur data untuk merepresentasikan Min-Heap
typedef struct
{
  HeapNodePtr data;
  int size;
  int capacity;
} MinHeap;

// Struktur data untuk merepresentasikan Max-Heap
typedef struct
{
  HeapNodePtr data;
  int size;
  int capacity;
} MaxHeap;

// Operasi Minheap
void minHeapInit(MinHeap *heap, HeapNodePtr data_buffer, int capacity);
void minHeapInsert(MinHeap *heap, HeapNode node);
void minHeapifyDown(MinHeap *heap, int idx);
void minHeapSort(MinHeap *heap);

// Operasi Maxheap
void maxHeapInit(MaxHeap *heap, HeapNodePtr data_buffer, int capacity);
void maxHeapInsert(MaxHeap *heap, HeapNode node);
void maxHeapifyDown(MaxHeap *heap, int idx);
void maxHeapSort(MaxHeap *heap);
void maxHeapReverse(MaxHeap *heap);

#endif // HEAP_H
