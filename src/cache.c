#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "cache.h"

/**
 * Allocate a cache entry
 */
struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)
{
    struct cache_entry *new_entry = malloc(sizeof *new_entry);
    new_entry->path = path;
    new_entry->content_type = content_type;
    new_entry->content_length = content_length;
    new_entry->content = content;

    return new_entry;
}

/**
 * Deallocate a cache entry
 */
void free_entry(struct cache_entry *entry)
{
    free(entry->path);
    free(entry->content_type);
    free(entry->content_length);
    free(entry->content);

    free(entry->prev);
    free(entry->next);
}

/**
 * Insert a cache entry at the head of the linked list
 */
void dllist_insert_head(struct cache *cache, struct cache_entry *ce)
{
    // Insert at the head of the list
    if (cache->head == NULL)
    {
        cache->head = cache->tail = ce;
        ce->prev = ce->next = NULL;
    }
    else
    {
        cache->head->prev = ce;
        ce->next = cache->head;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Move a cache entry to the head of the list
 */
void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)
{
    if (ce != cache->head)
    {
        if (ce == cache->tail)
        {
            // We're the tail
            cache->tail = ce->prev;
            cache->tail->next = NULL;
        }
        else
        {
            // We're neither the head nor the tail
            ce->prev->next = ce->next;
            ce->next->prev = ce->prev;
        }

        ce->next = cache->head;
        cache->head->prev = ce;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Removes the tail from the list and returns it
 * 
 * NOTE: does not deallocate the tail
 */
struct cache_entry *dllist_remove_tail(struct cache *cache)
{
    struct cache_entry *oldtail = cache->tail;

    cache->tail = oldtail->prev;
    cache->tail->next = NULL;

    cache->cur_size--;

    return oldtail;
}

/**
 * Create a new cache
 * 
 * max_size: maximum number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache *cache_create(int max_size, int hashsize)
{
    struct cache *cache = malloc(sizeof *cache);

    cache->index = hashtable_create(hashsize, NULL);

    cache->head = NULL;
    cache->tail = NULL;

    cache->max_size = max_size;
    cache->cur_size = 0;

    return cache;
}

void cache_free(struct cache *cache)
{
    struct cache_entry *cur_entry = cache->head;

    hashtable_destroy(cache->index);

    while (cur_entry != NULL)
    {
        struct cache_entry *next_entry = cur_entry->next;

        free_entry(cur_entry);

        cur_entry = next_entry;
    }
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)
{

    printf("lets PUT this in the cache");
    //Define new entry
    struct cache_entry *new_entry = alloc_entry(path, content_type, content, content_length);

    //Insert into the head of the doubly linked list
    dllist_insert_head(cache, new_entry);

    //Store the entry in the hashtable
    hashtable_put(cache->index, new_entry->path, new_entry);

    //Check to see if current elemnts is greater than max size allowed
    if (cache->cur_size > cache->max_size)
    {
        //Remove last element in cache
        struct cache_entry *old_tail = dllist_remove_tail(cache);

        //Delete from hashtable
        hashtable_delete(cache->index, old_tail->path);

        //Free memory
        free_entry(old_tail);
    }
}

/**
 * Retrieve an entry from the cache
 */
struct cache_entry *cache_get(struct cache *cache, char *path)
{

    printf("lets GET this from the cache");
    //Check to see if item exists in the cache
    struct cache_entry *entry = hashtable_get(cache->index, path);

    if (entry == NULL)
    {
        return NULL;
    }

    //Move entry to the head of the doubly linked list
    dllist_move_to_head(cache, entry);

    return entry;
}

// int main(void)
// {
//     printf("lets check this cache!\n");
//     struct cache *cache = cache_create(10, 0);
//     printf("created cache ");
//     printf(" with max size %d\n", cache->max_size);
//     cache_get(cache, "/index.html");
//     cache_put(cache, "/index.html", "text/html", "a file", 500);
// }