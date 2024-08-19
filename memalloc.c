#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef char ALIGN[16];

union header 
{
    struct 
    {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};

typedef union header header_t;

header_t *head;
header_t *tail;

pthread_mutex_t global_malloc_lock;

header_t *get_free_block(size_t size)
{
    header_t *current = head;
    while(current)
    {
		if(current->s.is_free && current->s.size >= size)
			return current;
            current = current->s.next;
        }
        return NULL;
}

void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
     
    if(!size)
        return NULL;
    
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
     
    if(header)
    {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
     
    total_size = sizeof(header) + size;
    block = (void*) sbrk(total_size);
     
    if(block == (void*) -1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
     
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
     
    if (!head)
		head = header;
        
    if (tail)
		tail->s.next = header;
     
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
     
    return (void*)(header + 1);
}

void *free(void* block)
{
	header_t *header;
	header_t *tmp;
	
	void *program_break;
	
	if(!block)
		return;
	
	pthread_mutex_lock(&global_malloc_lock);
	header = (header_t*)block - 1;
	
	program_break = (void*) sbrk(0);
	
	if((char*)block + header->s.size == program_break)
	{
		if(head == tail)
		{
			head = tail = NULL;
		} else {
			tmp = head;
			while(tmp)
			{
				if(tmp->s.next == tail)
				{
					tmp->s.next = NULL;
					tail = tmp;
				}
				tmp = tmp->s.next;
			}
		}
		sbrk(0 - sizeof(header_t) - header->s.size);
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
	
	header->s.is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
	return;
}

void *calloc(size_t num, size_t size)
{
	size_t total_size;
	void* block;
	
	if(!num || !size)
		return NULL;
	
	total_size = num * size;
	
	// Check multiplication overflow
	if(size != total_size / num)
		return NULL;
	
	block = malloc(total_size);
	
	if(!block)
		return NULL;
	
	memset(block, 0, total_size);
	return block;
}

void* realloc(void* block, size_t size)
{
	header_t *header;
	void *new_block;
	
	if(!block || !size)
		return malloc(size);
	
	header = (header_t*)block - 1;
	
	if(header->s.size >= size)
		return block;
	
	new_block = malloc(size);
	
	if(new_block)
	{
		memcpy(new_block, block, header->s.size);
		free(block);
	}
	
	return new_block;
}
