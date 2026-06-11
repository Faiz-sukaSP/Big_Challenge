#ifndef HEAP_H
#define HEAP_H

typedef struct HeapNode *HeapNodePtr;
typedef struct HeapNode {
  long long freq;
  int wordID;
} HeapNode;

typedef struct {
  HeapNodePtr data;
  int size;
  int capacity;
} MinHeap;

typedef struct {
  HeapNodePtr data;
  int size;
  int capacity;
} MaxHeap;

// MinHeap operations
void minHeapInit(MinHeap *heap, HeapNodePtr data_buffer, int capacity);
void minHeapInsert(MinHeap *heap, HeapNode node);
void minHeapifyDown(MinHeap *heap, int idx);
void minHeapSort(MinHeap *heap);

// MaxHeap operations
void maxHeapInit(MaxHeap *heap, HeapNodePtr data_buffer, int capacity);
void maxHeapInsert(MaxHeap *heap, HeapNode node);
void maxHeapifyDown(MaxHeap *heap, int idx);
void maxHeapSort(MaxHeap *heap);
void maxHeapReverse(MaxHeap *heap);

#endif // HEAP_H
