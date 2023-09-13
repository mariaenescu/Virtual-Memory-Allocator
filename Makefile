CC=gcc
CFLAGS=-Wall -Wextra -std=c99

TARGETS = vma

build: $(TARGETS)

vma: main.c vma.c
	$(CC) $(CFLAGS) main.c vma.c -o vma

run_vma: vma
	./vma

clean:
	rm -f $(TARGETS)