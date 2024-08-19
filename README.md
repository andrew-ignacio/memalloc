# memalloc

A simple memory allocator in C made to run in a Unix like OS.

The project implements:
	- malloc()
	- calloc()
	- realloc()
	- free()

*PS.: This is not an efficient and optimized memory allocator, it's just a toy project.*

## How to compile and run

Compile command:

```sh
gcc -o memalloc.so -fPIC -shared memalloc.c
```

The -fPIC and -shared flags tells the compiler that the compiled output has position-independent code and produces a shared object that can be dynamic linked.

To use the allocator set the variable LD_PRELOAD to the object produced by the compiler, this tells the OS to load the library before any other library on the system.

```sh
export LD_PRELOAD=$PWD/memalloc.so
```

After this any program that runs on the shell will use this implementation of memalloc:

```sh
ls
memalloc.c		memalloc.so		README.md
```

When done testing type:

```sh
unset LD_PRELOAD
```
## How it works

The header_t union defines a header to a memory area, the header points to the next memory allocated by the program:
```c
union header 
{
    struct 
    {
		size_t size;
        unsigned is_free;
        union header *next;
    } s;
    char[16] stub;
};

typedef union header header_t;
```
So the program not only allocate the data in the memory but also a header that contains information on the memory.

The whole logic of the program its just manipulating this structure to allocate memory with the srbk() system call or to free the memory previously allocated. Freeing the memory only happens when the block is located by the end of the stack, when the memory is located in the middle of the stack the memory just got marked as free (so it can be used to allocated some other data that fits in it).

## References:
- [Memory Allocators 101 - Write a simple memory allocator](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory)