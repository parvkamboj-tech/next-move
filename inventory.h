/*
 * inventory.h  —  Parv (C)
 * Inventory module: Hash Table (chaining with Linked Lists) + Binary Search Tree
 *
 * Hash table  -> fast lookup by product ID  (index = id % HASH_SIZE)
 * Linked list -> each bucket is a chain, handles collisions
 * BST         -> keeps products sorted by ID for ordered display
 */
#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_SIZE 10
#define NAME_LEN 40

typedef struct Product
{
    int id;
    char name[NAME_LEN];
    int quantity;
    int location;   /* warehouse location number (node in Siddhant's graph) */
} Product;

void inv_init(void);

/* returns 1 = added, 0 = duplicate ID, -1 = invalid input, -2 = out of memory */
int inv_add(int id, const char *name, int quantity, int location);

/* returns product or NULL; bucket and steps can be NULL if not needed */
Product *inv_search(int id, int *bucket, int *steps);

/* returns 1 = deleted, 0 = not found */
int inv_delete(int id);

/* change can be + or -; returns 1 = ok, 0 = not found, -1 = stock would go below 0 */
int inv_update_quantity(int id, int change);

void inv_display_sorted(void);        /* BST in-order traversal */
void inv_display_hash_table(void);    /* buckets with their chains */
int  inv_display_low_stock(int threshold);

int inv_count(void);
int inv_total_units(void);
int inv_low_stock_count(int threshold);

void inv_free(void);

#ifdef __cplusplus
}
#endif

#endif
