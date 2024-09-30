#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

// preallocate 16 megs
#define HUNK (16 * 1024 * 1024)

// made global for reading ease, (will be allocated
// prior to memory grabs)
int pos = 0;
uint8_t *pool = NULL;

// fetch memory from pool and advance pointer
int fetch(int size) {
  int loc = pos;
  pos += size;
  return loc;
}

// memory block
typedef struct block_s {
  int size;
  void *p;
} block_t;

typedef struct tile_s {
  int hash;
  int p;
  int used;
  block_t data;
  struct tile_s *next;
} tile_t;

#define HASH_TABLE_SIZE 1024

typedef struct hashmap_s {
  tile_t *table[HASH_TABLE_SIZE];
} hashmap_t;

int calc_hash(int v) {
  uint32_t x = (uint32_t) abs(v);
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x);
  return x % HASH_TABLE_SIZE;
//	return abs(v) % (TABLE_SIZE - 1);
}

hashmap_t map;
int recycle_count = 0;
int reserve_count = 0;

tile_t* heap_alloc(int size) {
  // 16 byte alignment
  int rem = size & 15;
  if (rem > 0)
    size += (16 - rem);

  int hash = calc_hash(size);
  tile_t *walk = map.table[hash];

  int lowest = INT_MAX;
  tile_t *find = NULL;
  // find a free tile with the best memory match
  while (walk) {
    if (!walk->used) {
      if (walk->data.size >= size && walk->data.size < lowest) {
        find = walk;
        lowest = walk->data.size;
      }
    }
    walk = walk->next;
  }
  if (find) {
    find->used = 1;
    recycle_count++;
    return find;
  }

  reserve_count++;
  if (map.table[hash] == NULL) {
    tile_t **tile = &map.table[hash];
    int loc = fetch(sizeof(tile_t) + size);
    *tile = (void*) &pool[loc];
    (*tile)->p = loc;
    (*tile)->data.size = size;
    (*tile)->data.p = &pool[loc + sizeof(tile_t)];
    (*tile)->used = 1;
    (*tile)->next = NULL;
    (*tile)->hash = hash;
    return *tile;
  }

  walk = map.table[hash];
  while (walk->next)
    walk = walk->next;
  int loc = fetch(sizeof(tile_t) + size);
  walk->next = (void*) &pool[loc];
  walk = walk->next;
  walk->p = loc;
  walk->hash = hash;
  walk->data.size = size;
  walk->data.p = &pool[loc + sizeof(tile_t)];
  walk->used = 1;
  walk->next = NULL;
  return walk;
}

// get memory
void* get_mem(tile_t *tile) {
  return tile->data.p;
}

// free memory
void free_mem(tile_t *tile) {
  tile->used = 0;
}


void heap_print_all() {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    if (!map.table[i])
      continue;
    printf("******\n");
    tile_t *walk = map.table[i];
    while (walk) {
      printf(" + %d - %d %s\n", walk->p, walk->data.size,
             walk->used ? "" : "(unused)");
      walk = walk->next;
    }
    printf("******\n");
  }
}

int main() {
  printf("hello world!\n");
  pool = malloc(HUNK);
  memset(pool, 0, HUNK);

  srand(0);
  tile_t *allocs[200] = { NULL };
  int requested = 0;
  for (int i = 0; i < 5000; i++) {
    int n = rand() % 200;
    if (allocs[n]) {
      free_mem(allocs[n]);
      allocs[n] = NULL;
    }
    int size = rand() % (4 * 1024) + 1; // allocate random sized memory between 1 and 4k bytes
    allocs[n] = heap_alloc(size);
    requested += size;
  }

  float recycle_rate = 100.0f * (float) recycle_count
      / (float) (reserve_count + recycle_count);
  printf("reservations v. recycles: %d v %d (recycle rate ~%.2f%%)\n",
         reserve_count, recycle_count, recycle_rate);
  printf("%f out of %f mbs reserved for use\n",
         ((float) pos) / (1024.0f * 1024.0f),
         (float) HUNK / (1024.0f * 1024.0f));
  printf("%f mbs actually requested\n",
         (float) requested / (1024.0f * 1024.0f));
  //heap_print_all();

  free(pool);
  printf("goodbye!\n");
  return 0;
}
