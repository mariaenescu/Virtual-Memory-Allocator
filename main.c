// Enescu Maria 311CA
#include "vma.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	char command[30];
	uint64_t address, size;
	arena_t *arena;
	while (1) {
		scanf("%s", command);
		if (strcmp(command, "ALLOC_ARENA") == 0) {
			scanf("%lu", &size);
			arena = alloc_arena(size);
		} else if (strcmp(command, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			break;
		} else if (strcmp(command, "ALLOC_BLOCK") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			alloc_block(arena, address, size);
		} else if (strcmp(command, "FREE_BLOCK") == 0) {
			scanf("%lu", &address);
			free_block(arena, address);
		} else if (strcmp(command, "READ") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			read(arena, address, size);
		} else if (strcmp(command, "WRITE") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			char *data = malloc((size + 1) * sizeof(char));
			uint64_t i = 0;
			char c;
			scanf("%c", &c);
			while (i != size) {
				scanf("%c", &data[i]);
				i++;
			}
			data[size] = '\0';
			write(arena, address, size, data);
			free(data);
		} else if (strcmp(command, "PMAP") == 0) {
			pmap(arena);
		} else if (strcmp(command, "MPROTECT") == 0) {
			scanf("%lu", &address);
			char command2[50];
			int i = 0;
			scanf("%c", &command2[i]);
			while (command2[i] != '\n') {
				i++;
				scanf("%c", &command2[i]);
			}
			command2[i] = '\0';
			mprotect(arena, address, command2);
		} else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
