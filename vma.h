// Enescu Maria 311CA
#include <inttypes.h>
#include <stddef.h>

typedef struct list_t list_t;
typedef struct list_t {
	void *head;
	size_t data_size;
	uint64_t size;
} list_t;

typedef struct block_t block_t;
typedef struct block_t {
	uint64_t start_address;
	size_t size;
	void *miniblock_list;
	block_t *prev, *next;
} block_t;

typedef struct miniblock_t miniblock_t;
typedef struct miniblock_t {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
	miniblock_t *prev, *next;
} miniblock_t;

typedef struct arena_t arena_t;
typedef struct arena_t {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

miniblock_t *create_miniblock(const uint64_t address, size_t size);
block_t *create_block(const uint64_t address, const uint64_t size,
					  int alloc_miniblock);
arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);
void add(block_t *block, miniblock_t *miniblock, miniblock_t *new_miniblock);
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void unique_miniblock(arena_t *arena, block_t *block, miniblock_t *miniblock);
void remove_miniblock(arena_t *arena, block_t *block, miniblock_t *miniblock);
void aux_free_block(arena_t *arena, block_t *block, miniblock_t *miniblock);
void free_block(arena_t *arena, const uint64_t address);
void aux_read(miniblock_t *curr_miniblock, uint64_t address, uint64_t n);
void read(arena_t *arena, uint64_t address, uint64_t size);
void aux_write(miniblock_t *curr_miniblock, uint64_t address,
			   int8_t *data, const uint64_t size, uint64_t index);
void write(arena_t *arena, const uint64_t address,  const uint64_t size,
		   int8_t *data);
char *str_perm(uint8_t nr);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
