// Enescu Maria 311CA
#include "vma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
	Functie folosita pentru a crea o lista dublu intantuita.
**/
list_t *create_list(unsigned int data_size)
{
	list_t *list;
	list = malloc(sizeof(list_t));
	list->data_size = data_size;
	list->size = 0;
	list->head = NULL;
	return list;
}

/**
	Functie folosita pentru a initializa un miniblock.
**/
miniblock_t *create_miniblock(const uint64_t address, size_t size)
{
	miniblock_t *new_miniblock = malloc(sizeof(miniblock_t));
	new_miniblock->size = size;
	new_miniblock->start_address = address;
	new_miniblock->rw_buffer = NULL;
	new_miniblock->perm = 6;
	new_miniblock->prev = NULL;
	new_miniblock->next = NULL;
	return new_miniblock;
}

/**
	Functie folosita pentru a initializa un block.
	* Daca parametrul alloc_miniblock != 0 => aloc un miniblock
	odata cu crearea block-ului;
    * Daca parametrul alloc_miniblock == 0 => nu aloc miniblock-ul
	odata cu crearea block-ului;
**/
block_t *create_block(const uint64_t address, const uint64_t size,
					  int alloc_miniblock)
{
	block_t *new_block = malloc(sizeof(block_t));
	new_block->size = size;
	if (alloc_miniblock != 0)
		new_block->miniblock_list = create_miniblock(address, size);
	new_block->start_address = address;
	new_block->prev = NULL;
	new_block->next = NULL;
	return new_block;
}

/**
	Functie folosita pentru a crea arena fictiva, respectiv lista pentru
	block-uri.
**/
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena;
	arena = malloc(sizeof(arena_t));
	if (!arena)
		return NULL;
	arena->arena_size = size;
	arena->alloc_list = create_list(arena->arena_size);
	return arena;
}

/**
	Functie folosita pentru eliberarea tuturor resurselor folosite.
**/
void dealloc_arena(arena_t *arena)
{
	block_t *block = arena->alloc_list->head;
	block_t *tmp_block;
	while (block) {
		miniblock_t *miniblock = block->miniblock_list;
		miniblock_t *tmp;
		while (miniblock) {
			if (miniblock->rw_buffer)
				free(miniblock->rw_buffer);
			tmp = miniblock;
			miniblock = miniblock->next;
			free(tmp);
		}
		tmp_block = block;
		block = block->next;
		free(tmp_block);
	}
	free(arena->alloc_list);
	free(arena);
}

/**
	Functie auxiliara ce contine o parte din continutul functiei alloc_block.
**/
void aux_alloc_block(block_t *block, miniblock_t *miniblock,
					 miniblock_t *new_miniblock)
{
	while (miniblock->next)
		miniblock = miniblock->next;
	miniblock->next = new_miniblock;
	new_miniblock->prev = miniblock;
	new_miniblock->next = NULL;
	if (block->next && block->next->start_address ==
		block->start_address + block->size) {
		block_t *tmp = block->next;
		block->size += tmp->size;
		new_miniblock->next = tmp->miniblock_list;
		((miniblock_t *)tmp->miniblock_list)->prev = new_miniblock;
		block->next = tmp->next;
		if (tmp->next)
			tmp->next->prev = block;
		free(tmp);
	}
}

/**
	Functie folosita pentru:
	* Adaugarea unui block in cazul in care este lipsit de adiacenta;
	* Adaugarea listei de minblock-uri al block-ului din dreapta la
	block-ul din stanga, practic zonele adiacente din memorie vor fi in
	acelasi block. Pentru acest caz folosesc o functie
	auxiliara(aux_alloc_block);
**/
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}
	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	block_t *curr_block = (block_t *)arena->alloc_list->head;
	if (!curr_block) {
		block_t *tmp = create_block(address, size, 1);
		arena->alloc_list->head = tmp;
		return;
	}
	block_t *aux_curr = curr_block;
	while (curr_block) {
		if (curr_block->start_address == address) {
			printf("This zone was already allocated.\n");
			return;
		} else if (address < curr_block->start_address) {
			if (address + size > curr_block->start_address) {
				printf("This zone was already allocated.\n");
			} else if (address + size == curr_block->start_address) {
				curr_block->start_address = address;
				curr_block->size += size;
				miniblock_t *new_miniblock = create_miniblock(address, size);
				miniblock_t *first_miniblock = curr_block->miniblock_list;
				new_miniblock->next = first_miniblock;
				first_miniblock->prev = new_miniblock;
				curr_block->miniblock_list = new_miniblock;
			} else {
				block_t *new_block = create_block(address, size, 1);
				new_block->next = curr_block;
				new_block->prev = curr_block->prev;
				if (curr_block->prev)
					curr_block->prev->next = new_block;
				curr_block->prev = new_block;
				if (arena->alloc_list->head == curr_block)
					arena->alloc_list->head = new_block;
			}
			return;
		} else if (curr_block->start_address + curr_block->size == address) {
			curr_block->size += size;
			miniblock_t *new_miniblock = create_miniblock(address, size);
			miniblock_t *curr_miniblock = curr_block->miniblock_list;
			aux_alloc_block(curr_block, curr_miniblock, new_miniblock);
			return;
		} else if (curr_block->next && curr_block->next->start_address ==
				   address + size) {
			curr_block->next->start_address = address;
			curr_block->next->size += size;
			miniblock_t *new_miniblock = create_miniblock(address, size);
			miniblock_t *tmp = (miniblock_t *)curr_block->next->miniblock_list;
			tmp->prev = new_miniblock;
			new_miniblock->next = tmp;
			curr_block->next->miniblock_list = new_miniblock;
			return;
		} else if (curr_block->start_address > address) {
			aux_curr = curr_block;
			break;
		}
		aux_curr = curr_block;
		curr_block = curr_block->next;
	}
	if (aux_curr->start_address + aux_curr->size <= address) {
		block_t *new_block = create_block(address, size, 1);
		block_t *tmp = aux_curr->next;
		aux_curr->next = new_block;
		new_block->prev = aux_curr;
		if (tmp) {
			tmp->prev = new_block;
			new_block->next = tmp;
		}
	} else {
		printf("This zone was already allocated.\n");
	}
}

/**
	Functia sterge block-ul din lista acand acesta contine un singur miniblock.
	Functie regasita in functia aux_free_block.
**/
void unique_miniblock(arena_t *arena, block_t *block, miniblock_t *miniblock)
{
	if (block->next)
		block->next->prev = block->prev;
	if (block->prev)
		block->prev->next = block->next;
	if (arena->alloc_list->head == block)
		arena->alloc_list->head = block->next;
	block->prev = NULL;
	block->next = NULL;
	if (miniblock->rw_buffer)
		free(miniblock->rw_buffer);
	free(miniblock);
	free(block);
}

/**
	Functia sterge un minilock din lista de miniblock-uri cand block-ul are
	mai multe miniblock-uri.
    * Daca miniblock-l este pe prima sau pe ultima pozitie din lista => listei
	de miniblock-uri i se pastreaza structura, i se refac legaturile
	corespunzator si se elimina miniblock-ul;
    * Daca miniblock-ul se afla in mijlocul listei => se muta tot ce
    este dupa miniblock intr-un alt block;
	Functie regasita in functia aux_free_block.
**/
void remove_miniblock(arena_t *arena, block_t *block, miniblock_t *miniblock)
{
	if (!miniblock->prev) {
		block->miniblock_list = miniblock->next;
		block->start_address += miniblock->size;
		miniblock->next->prev = NULL;
	} else if (!miniblock->next) {
		miniblock->prev->next = NULL;
	} else {
		unsigned int size = 0;
		miniblock_t *aux = miniblock->next;
		while (aux) {
			size += aux->size;
			aux = aux->next;
		}
		uint64_t address_mini =
		miniblock->next->start_address;
		block_t *new_block = create_block(address_mini, size, 0);
		new_block->miniblock_list = miniblock->next;
		miniblock->next->prev = NULL;
		new_block->size = size;
		block_t *tmp = block->next;
		new_block->next = tmp;
		block->next = new_block;
		new_block->prev = block;
		if (tmp)
			tmp->prev = new_block;
		block->size -= new_block->size;
		miniblock->prev->next = NULL;
		miniblock->next = NULL;
		miniblock->prev = NULL;
	}
}

/**
	Functie auxiliara ce continua continutul functiei free_block.
**/
void aux_free_block(arena_t *arena, block_t *block, miniblock_t *miniblock)
{
	if (miniblock->size == block->size &&
		!miniblock->prev && !miniblock->next) {
		unique_miniblock(arena, block, miniblock);
	} else {
		remove_miniblock(arena, block, miniblock);
		block->size -= miniblock->size;
		if (miniblock->rw_buffer)
			free(miniblock->rw_buffer);
		free(miniblock);
	}
}

/**
	Functie folosita pentru eliberarea unui miniblock.
**/
void free_block(arena_t *arena, const uint64_t address)
{
	block_t *curr_block = arena->alloc_list->head;
	while (curr_block) {
		if (curr_block->start_address <= address &&
			address <= curr_block->start_address + curr_block->size) {
			miniblock_t *curr_miniblock = curr_block->miniblock_list;
			while (curr_miniblock) {
				if (curr_miniblock->start_address == address) {
					aux_free_block(arena, curr_block, curr_miniblock);
					return;
				}
				curr_miniblock = curr_miniblock->next;
			}
		}
		curr_block = curr_block->next;
	}
	printf("Invalid address for free.\n");
}

/**
	Functie auxiliara ce continua continutul functiei read.
**/
void aux_read(miniblock_t *curr_miniblock, uint64_t address, uint64_t n)
{
	int print = 1;
	int start_read = address - curr_miniblock->start_address;
	char result[n], tmp[n];
	result[0] = '\0';
	while (n > 0 && curr_miniblock &&
		   curr_miniblock->rw_buffer) {
		char *s = (char *)curr_miniblock->rw_buffer;
		if ((curr_miniblock->perm & 4) == 0) {
			print = 0;
			break;
		}
		if (n >= curr_miniblock->size) {
			s[curr_miniblock->size] = '\0';
			sprintf(tmp, "%.*s", (int)curr_miniblock->size -
					start_read, s + start_read);
			n -= (curr_miniblock->size - start_read);
			strcat(result, tmp);
		} else {
			sprintf(tmp, "%.*s", (int)n, s + start_read);
			strcat(result, tmp);
			break;
		}
		start_read = 0;
		curr_miniblock = curr_miniblock->next;
	}
	if (print == 1)
		printf("%s\n", result);
	else
		printf("Invalid permissions for read.\n");
}

/**
	Functie folosita pentru afisarea continutului unui/ unor miniblock-uri
	de la o adresa specificata.
**/
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	block_t *curr_block = arena->alloc_list->head;
	while (curr_block) {
		if (curr_block->start_address <= address && address <=
			curr_block->start_address + curr_block->size) {
			miniblock_t *curr_miniblock = curr_block->miniblock_list;
			while (curr_miniblock) {
				if (curr_miniblock->start_address == address ||
					(curr_miniblock->start_address < address &&
					curr_miniblock->start_address + curr_miniblock->size >
					address)) {
					uint64_t read_space = curr_block->size -
										 (address - curr_block->start_address);
					if (read_space < size) {
						printf("Warning: size was bigger than the block size");
						printf(". Reading %ld characters.\n", read_space);
					}
					if ((curr_miniblock->perm & 4) == 0) {
						printf("Invalid permissions for read.\n");
						return;
					}
					uint64_t n = size;
					aux_read(curr_miniblock, address, n);
					return;
				}
				curr_miniblock = curr_miniblock->next;
			}
		}
		curr_block = curr_block->next;
	}
	printf("Invalid address for read.\n");
}

/**
	Functie auxiliara ce continua continutul functiei write.
**/
void aux_write(miniblock_t *curr_miniblock, uint64_t address,
			   int8_t *data, const uint64_t size, uint64_t index)
{
	while (index < size) {
		curr_miniblock = curr_miniblock->next;
		if (!curr_miniblock) {
			printf("Warning: size was bigger than the block ");
			printf("size. Writing %ld characters.\n", index);
			break;
		}
		if ((curr_miniblock->perm & 2) == 0) {
			printf("Invalid permissions for write.\n");
			return;
		}
		if (size - index >= curr_miniblock->size) {
			int i = (curr_miniblock->size + 1);
			curr_miniblock->rw_buffer =
			malloc(i * sizeof(char));
			memcpy(((char *)curr_miniblock->rw_buffer),
				   data + index, curr_miniblock->size);
			index += curr_miniblock->size;
		} else {
			int i = (size - index + 1);
			curr_miniblock->rw_buffer =
			malloc(i * sizeof(char));
			memcpy(((char *)curr_miniblock->rw_buffer),
				   data + index, (size - index));
			index += (size - index);
		}
	}
}

/**
	Functie folosita pentru scrierea de continut la o adresa specificata.
**/
void write(arena_t *arena, const uint64_t address, const uint64_t size,
		   int8_t *data)
{
	block_t *curr_block = arena->alloc_list->head;
	while (curr_block) {
		if (curr_block->start_address <= address && address <=
			curr_block->start_address + curr_block->size) {
			miniblock_t *curr_miniblock = curr_block->miniblock_list;
			unsigned int rest_size = 0;
			while (curr_miniblock) {
				if (curr_miniblock->start_address == address) {
					if ((curr_miniblock->perm & 2) == 0) {
						printf("Invalid permissions for write.\n");
						return;
					}
					uint64_t index = 0;
					if (size >= curr_miniblock->size) {
						int i = (curr_miniblock->size + 1);
						curr_miniblock->rw_buffer = malloc(i * sizeof(char));
						memcpy(((char *)curr_miniblock->rw_buffer), data,
							   curr_miniblock->size);
						index = curr_miniblock->size;
					} else {
						int i = (size + 1);
						curr_miniblock->rw_buffer = malloc(i * sizeof(char));
						memcpy(((char *)curr_miniblock->rw_buffer), data,
							   size);
						index = size;
					}
					aux_write(curr_miniblock, address, data, size, index);
					return;
				}
				curr_miniblock = curr_miniblock->next;
			}
		}
		curr_block = curr_block->next;
	}
	printf("Invalid address for write.\n");
}

/**
	Functie folosita pentru afisarea corespunzatoare a permisiunilor
	dupa bitul dat.
**/
char *str_perm(uint8_t nr)
{
	char *s = malloc(4 * sizeof(char));
	s[0] = nr & 4 ? 'R' : '-';
	s[1] = nr & 2 ? 'W' : '-';
	s[2] = nr & 1 ? 'X' : '-';
	s[3] = '\0';
	return s;
}

/**
	Functie folosita pentru afisarea informatiilor corespunzatoare fiecarei
	modificari prin care a trecut lista de block-uri si cea de miniblock-uri
	in formatul aferent.
**/
void pmap(const arena_t *arena)
{
	block_t *curr_block = (block_t *)arena->alloc_list->head;
	size_t total_size = 0;
	size_t nr_blocks = 0;
	size_t nr_miniblocks = 0;
	while (curr_block) {
		total_size += curr_block->size;
		nr_blocks++;
		miniblock_t *curr_miniblock = curr_block->miniblock_list;
		while (curr_miniblock) {
			nr_miniblocks++;
			curr_miniblock = curr_miniblock->next;
		}
		curr_block = curr_block->next;
	}
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	printf("Free memory: 0x%lX bytes\n", arena->arena_size - total_size);
	printf("Number of allocated blocks: %ld\n", nr_blocks);
	printf("Number of allocated miniblocks: %ld\n", nr_miniblocks);
	curr_block = (block_t *)arena->alloc_list->head;
	size_t i = 0;
	while (curr_block) {
		i++;
		printf("\n");
		printf("Block %ld begin\n", i);
		size_t j = 1;
		printf("Zone: 0x%lX - 0x%lX\n", curr_block->start_address,
			   curr_block->start_address + curr_block->size);
		miniblock_t *curr_miniblock = curr_block->miniblock_list;
		while (curr_miniblock) {
			char *s = str_perm(curr_miniblock->perm);
			printf("Miniblock %ld:\t\t0x%lX\t\t-\t\t0x%lX\t\t| %s\n", j,
				   curr_miniblock->start_address,
				   curr_miniblock->start_address + curr_miniblock->size, s);
			j++;
			free(s);
			curr_miniblock = curr_miniblock->next;
		}
		printf("Block %ld end\n", i);
		curr_block = curr_block->next;
	}
}

/**
	Functie folosita pentru schimbarea permisiunilor.
	Permisiunile sunt stocate într-un sir de caractere tokens,
	dupa care se cauta cuvintele cheie. Daca unul dintre aceste
	cuvinte cheie este gasit in tokens, se setează bitul corespunzator
	in variabila v.
**/
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	if (address > arena->arena_size) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	block_t *curr_block = arena->alloc_list->head;
	while (curr_block) {
		if (curr_block->start_address <= address && address <=
		curr_block->start_address + curr_block->size) {
			miniblock_t *curr_miniblock = curr_block->miniblock_list;
			while (curr_miniblock) {
				if (curr_miniblock->start_address == address) {
					uint8_t v = 0;
					char *tokens = (char *)permission;
					if (strstr(tokens, "PROT_READ") != 0)
						v |= 4;
					if (strstr(tokens, "PROT_WRITE") != 0)
						v |= 2;
					if (strstr(tokens, "PROT_EXEC") != 0)
						v |= 1;
					curr_miniblock->perm = v;
					return;
				}
				curr_miniblock = curr_miniblock->next;
			}
		}
		curr_block = curr_block->next;
	}
	printf("Invalid address for mprotect.\n");
}
