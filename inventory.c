/*
 * inventory.c  —  Parv (C)
 * Hash Table + Linked List chaining + Binary Search Tree
 *
 * Run alone:   gcc -DSTANDALONE inventory.c -o parv_inventory
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"

/* ---------- Linked list node for hash table chains ---------- */
typedef struct HashNode
{
    Product *data;
    struct HashNode *next;
} HashNode;

/* ---------- BST node ---------- */
typedef struct TreeNode
{
    Product *data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static HashNode *table[HASH_SIZE];
static TreeNode *root = NULL;
static int productCount = 0;

static int hashFunction(int id)
{
    return id % HASH_SIZE;
}

void inv_init(void)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++)
        table[i] = NULL;
    root = NULL;
    productCount = 0;
}

/* ===================== ADD ===================== */
int inv_add(int id, const char *name, int quantity, int location)
{
    Product *p;
    HashNode *h;
    TreeNode *t;
    int index;

    if (id <= 0 || quantity < 0 || name == NULL || name[0] == '\0')
        return -1;

    if (inv_search(id, NULL, NULL) != NULL)
        return 0;

    p = (Product *)malloc(sizeof(Product));
    h = (HashNode *)malloc(sizeof(HashNode));
    t = (TreeNode *)malloc(sizeof(TreeNode));

    if (p == NULL || h == NULL || t == NULL)
    {
        free(p);
        free(h);
        free(t);
        return -2;
    }

    p->id = id;
    strncpy(p->name, name, NAME_LEN - 1);
    p->name[NAME_LEN - 1] = '\0';
    p->quantity = quantity;
    p->location = location;

    /* insert at head of the bucket's linked list */
    index = hashFunction(id);
    h->data = p;
    h->next = table[index];
    table[index] = h;

    /* insert into BST (same Product pointer is shared) */
    t->data = p;
    t->left = NULL;
    t->right = NULL;

    if (root == NULL)
    {
        root = t;
    }
    else
    {
        TreeNode *cur = root;
        while (1)
        {
            if (id < cur->data->id)
            {
                if (cur->left == NULL) { cur->left = t; break; }
                cur = cur->left;
            }
            else
            {
                if (cur->right == NULL) { cur->right = t; break; }
                cur = cur->right;
            }
        }
    }

    productCount++;
    return 1;
}

/* ===================== SEARCH (hash table) ===================== */
Product *inv_search(int id, int *bucket, int *steps)
{
    int index;
    int count = 0;
    HashNode *cur;

    if (id <= 0)
    {
        if (bucket) *bucket = -1;
        if (steps) *steps = 0;
        return NULL;
    }

    index = hashFunction(id);
    cur = table[index];

    while (cur != NULL)
    {
        count++;
        if (cur->data->id == id)
            break;
        cur = cur->next;
    }

    if (bucket) *bucket = index;
    if (steps) *steps = count;

    return (cur != NULL) ? cur->data : NULL;
}

/* ===================== DELETE ===================== */
static TreeNode *bstDelete(TreeNode *node, int id)
{
    TreeNode *temp;

    if (node == NULL)
        return NULL;

    if (id < node->data->id)
        node->left = bstDelete(node->left, id);
    else if (id > node->data->id)
        node->right = bstDelete(node->right, id);
    else
    {
        if (node->left == NULL)
        {
            temp = node->right;
            free(node);
            return temp;
        }
        if (node->right == NULL)
        {
            temp = node->left;
            free(node);
            return temp;
        }

        /* two children: copy in-order successor, then delete it */
        temp = node->right;
        while (temp->left != NULL)
            temp = temp->left;

        node->data = temp->data;
        node->right = bstDelete(node->right, temp->data->id);
    }
    return node;
}

int inv_delete(int id)
{
    int index;
    HashNode *cur, *prev = NULL;
    Product *p;

    if (id <= 0)
        return 0;

    index = hashFunction(id);
    cur = table[index];

    while (cur != NULL && cur->data->id != id)
    {
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL)
        return 0;

    p = cur->data;

    /* unlink from chain */
    if (prev == NULL)
        table[index] = cur->next;
    else
        prev->next = cur->next;
    free(cur);

    /* remove from tree, then free the product itself */
    root = bstDelete(root, id);
    free(p);

    productCount--;
    return 1;
}

/* ===================== UPDATE ===================== */
int inv_update_quantity(int id, int change)
{
    Product *p = inv_search(id, NULL, NULL);

    if (p == NULL)
        return 0;
    if (p->quantity + change < 0)
        return -1;

    p->quantity += change;
    return 1;
}

/* ===================== DISPLAY ===================== */
static void inorderPrint(TreeNode *node)
{
    if (node == NULL)
        return;
    inorderPrint(node->left);
    printf("%-8d %-25s %8d %10d %8d\n",
           node->data->id, node->data->name, node->data->quantity,
           node->data->location, hashFunction(node->data->id));
    inorderPrint(node->right);
}

void inv_display_sorted(void)
{
    if (root == NULL)
    {
        printf("\nInventory is empty!\n");
        return;
    }
    printf("\n%-8s %-25s %8s %10s %8s\n", "ID", "Name", "Qty", "Location", "Bucket");
    printf("---------------------------------------------------------------\n");
    inorderPrint(root);
}

void inv_display_hash_table(void)
{
    int i;
    HashNode *cur;

    printf("\n===== HASH TABLE (index = id %% %d) =====\n", HASH_SIZE);
    for (i = 0; i < HASH_SIZE; i++)
    {
        printf("[%d] ", i);
        cur = table[i];
        while (cur != NULL)
        {
            printf("-> %d(%s) ", cur->data->id, cur->data->name);
            cur = cur->next;
        }
        printf("-> NULL\n");
    }
}

static int lowStockPrint(TreeNode *node, int threshold, int print)
{
    int count;
    if (node == NULL)
        return 0;
    count = lowStockPrint(node->left, threshold, print);
    if (node->data->quantity < threshold)
    {
        if (print)
            printf("%-8d %-25s %8d\n", node->data->id, node->data->name, node->data->quantity);
        count++;
    }
    count += lowStockPrint(node->right, threshold, print);
    return count;
}

int inv_display_low_stock(int threshold)
{
    int count;
    printf("\n===== LOW STOCK (below %d units) =====\n", threshold);
    count = lowStockPrint(root, threshold, 1);
    if (count == 0)
        printf("All products are sufficiently stocked.\n");
    return count;
}

int inv_low_stock_count(int threshold)
{
    return lowStockPrint(root, threshold, 0);
}

int inv_count(void)
{
    return productCount;
}

static int sumUnits(TreeNode *node)
{
    if (node == NULL)
        return 0;
    return node->data->quantity + sumUnits(node->left) + sumUnits(node->right);
}

int inv_total_units(void)
{
    return sumUnits(root);
}

/* ===================== FREE MEMORY ===================== */
static void freeTree(TreeNode *node)
{
    if (node == NULL)
        return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

void inv_free(void)
{
    int i;
    HashNode *cur, *next;

    freeTree(root);
    root = NULL;

    for (i = 0; i < HASH_SIZE; i++)
    {
        cur = table[i];
        while (cur != NULL)
        {
            next = cur->next;
            free(cur->data);   /* each product is freed exactly once, here */
            free(cur);
            cur = next;
        }
        table[i] = NULL;
    }
    productCount = 0;
}

/* ===================== STANDALONE MENU (Parv's demo) ===================== */
#ifdef STANDALONE
static int readInt(const char *prompt)
{
    int x, c;
    printf("%s", prompt);
    while (scanf("%d", &x) != 1)
    {
        while ((c = getchar()) != '\n' && c != EOF);
        if (c == EOF) exit(0);
        printf("Please enter a number: ");
    }
    return x;
}

int main(void)
{
    int choice, id, qty, loc, result, bucket, steps;
    char name[NAME_LEN];
    Product *p;

    inv_init();

    do
    {
        printf("\n\n===== INVENTORY SYSTEM (Parv) =====");
        printf("\n1. Add Product");
        printf("\n2. Search Product");
        printf("\n3. Update Quantity");
        printf("\n4. Delete Product");
        printf("\n5. Display Sorted (BST)");
        printf("\n6. Display Hash Table");
        printf("\n7. Low Stock Report");
        printf("\n8. Exit");
        choice = readInt("\n\nEnter Choice: ");

        switch (choice)
        {
        case 1:
            id = readInt("\nEnter Product ID: ");
            printf("Enter Product Name: ");
            if (scanf(" %39[^\n]", name) != 1) return 0;
            qty = readInt("Enter Quantity: ");
            loc = readInt("Enter Location Number: ");
            result = inv_add(id, name, qty, loc);
            if (result == 1) printf("\nProduct Added Successfully! (bucket %d)\n", id % HASH_SIZE);
            else if (result == 0) printf("\nProduct ID already exists!\n");
            else printf("\nInvalid input!\n");
            break;

        case 2:
            id = readInt("\nEnter Product ID: ");
            p = inv_search(id, &bucket, &steps);
            if (p != NULL)
                printf("\nFound: %s | Qty: %d | Location: %d | Bucket %d, %d node(s) checked\n",
                       p->name, p->quantity, p->location, bucket, steps);
            else
                printf("\nProduct Not Found!\n");
            break;

        case 3:
            id = readInt("\nEnter Product ID: ");
            qty = readInt("Enter change (+ to add, - to remove): ");
            result = inv_update_quantity(id, qty);
            if (result == 1) printf("\nQuantity Updated!\n");
            else if (result == 0) printf("\nProduct Not Found!\n");
            else printf("\nNot enough stock to remove that many!\n");
            break;

        case 4:
            id = readInt("\nEnter Product ID: ");
            if (inv_delete(id)) printf("\nProduct Deleted!\n");
            else printf("\nProduct Not Found!\n");
            break;

        case 5: inv_display_sorted(); break;
        case 6: inv_display_hash_table(); break;
        case 7: inv_display_low_stock(10); break;
        case 8: printf("\nExiting...\n"); break;
        default: printf("\nInvalid Choice!");
        }
    } while (choice != 8);

    inv_free();
    return 0;
}
#endif
