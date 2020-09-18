/*
 * Small module with the most interesting storage structures
 * inside the Linux kernel:
 *
 * - Linked list.
 * - Hash table.
 * - Red black tree.
 * - Radix tree.
 * - XArray.
 * 
*/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/hashtable.h>
#include <linux/rbtree.h>
#include <linux/radix-tree.h>
#include <linux/xarray.h>

static char *int_str;

static int elems = 0;

#define MAX_SIZE 2048

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Bilbao");
MODULE_DESCRIPTION("Showcase data structures");

module_param(int_str, charp, S_IRUSR | S_IRGRP | S_IROTH);

MODULE_PARM_DESC(int_str, "A comma-separated list of integers");

/* [0] Linked list --------------------------------------------------------- */

static LIST_HEAD(mylist);

struct entry {
        int val;
        struct list_head list;
};

/* [1] Hash Table (32 buckets, 2^5) ---------------------------------------- */

static DECLARE_HASHTABLE(hash_table,5);

struct hash_entry {
        int val;
        struct hlist_node hlist;
};

/* [2] Red Black Tree (semi-balanced binary tree)------------------------------ 
   The path from the root to the farthest leaf is no more than twice as long as
   the path from the root to the nearest leaf.
*/

struct rb_root redblack_root = RB_ROOT;

struct redblack_entry {
        struct rb_node node;
        int val;
};

/* [3] Radix Tree ---------------------------------------------------------- */

static RADIX_TREE(radix_tree, GFP_KERNEL);

struct radix_entry {
        int val;
};

/* [4] XArray ---------------------------------------------------------------*/
static DEFINE_XARRAY(xarray);

struct xarray_entry {
        int val;
};

/* PROC FILE SYSTEM ENTRY */
static struct proc_dir_entry *proc_proj2;

int add_value(int read,int first,int val,char members[])
{
        int aux = 0;

        if (read + 3 < MAX_SIZE - 1){
                if (!first){
                        aux += sprintf(&members[read],",");
                        printk(KERN_CONT ",");
                }
                aux += sprintf(&members[aux+read]," %i",val);
                printk(KERN_CONT " %i",val);
        }

        return aux;
}

/* Auxiliar function - return number of elements*/
int print_lists(char members[])
{
        struct entry* item                = NULL;
        struct list_head* cur_node        = NULL;
        struct hash_entry* cur_hash_node  = NULL;
        struct rb_node *redblack_node     = NULL;
        struct redblack_entry *rb_entry   = NULL;
        struct radix_entry *radix_entry   = NULL;
        struct xarray_entry *xarray_entry = NULL;
        int read = 0, first = 1, key = 0;
        char* aux_linked   = (char*) "Linked list:";
        char* aux_hash     = (char*) "Hash table:";
        char* aux_redblack = (char*) "Red-black tree:";
        char* aux_radix    = (char*) "Radix tree:";
        char* aux_xarray   = (char*) "XArray:";

        /* [0] Linked list ================================================= */

        while ((members[read++]=*aux_linked)!= ':' && read < MAX_SIZE - 1) {++aux_linked;}
        printk(KERN_CONT "Linked list: ");

        list_for_each(cur_node, &mylist) { /* while cur_node != mylist*/
                item = list_entry(cur_node,struct entry, list);
                read += add_value(read,first,item->val,members);
                first = 0;
        }

        members[read++] = '\n';
        printk(KERN_INFO "");
        first = 1;

        /* [1] Hash table ================================================== */

        while ((members[read++]=*aux_hash)!= ':' && read < MAX_SIZE - 1) {++aux_hash;}
        printk(KERN_CONT "Hash table: ");

        hash_for_each(hash_table,key,cur_hash_node,hlist) {
                read += add_value(read,first,cur_hash_node->val,members);
                first = 0;
        }

        members[read++] = '\n';
        printk(KERN_INFO "");
        first = 1;

        /* [2] Red-back tree =============================================== */

        while ((members[read++]=*aux_redblack)!= ':' && read < MAX_SIZE - 1) {++aux_redblack;}
        printk(KERN_CONT "Red-black tree: ");

        if (likely(!RB_EMPTY_ROOT(&redblack_root))) {
                redblack_node = rb_first(&redblack_root);
        }

        while (redblack_node){
                rb_entry = rb_entry(redblack_node,struct redblack_entry,node);
                read += add_value(read,first,rb_entry->val,members);
                redblack_node = rb_next(redblack_node);
                first = 0;
        }

        members[read++] = '\n';
        printk(KERN_INFO "");
        first = 1;

        /* [3] Radix tree ================================================== */

        while ((members[read++]=*aux_radix)!= ':' && read < MAX_SIZE - 1) {++aux_radix;}
        printk(KERN_CONT "Radix tree: ");

        key = 0;
        radix_entry = radix_tree_lookup(&radix_tree,key++);

        while (radix_entry) {
                read += add_value(read,first,radix_entry->val,members);
                radix_entry = radix_tree_lookup(&radix_tree,key++);
                first = 0;
                if (key > elems) break;
        }

        members[read++] = '\n';
        printk(KERN_INFO "");
        first = 1;

        /* [4] XArray ====================================================== */

        while ((members[read++]=*aux_xarray)!= ':' && read < MAX_SIZE - 1) {++aux_xarray;}
        printk(KERN_CONT "XArray: ");

        key = 0;
        xarray_entry = xa_load(&xarray,key++);

        while (xarray_entry){
                read += add_value(read,first,xarray_entry->val,members);
                xarray_entry = xa_load(&xarray,key++);
                first = 0;
                if (key > elems) break;
        }

        members[read++] = '\n';
        printk(KERN_INFO "");

        return read;
}

ssize_t proj2_read(struct file *filep, char __user *buf, size_t len, loff_t *off){

        char kbuf[MAX_SIZE];
        int read = 0;

        /* Previously invoked */
        if ((*off) > 0) return 0;

        read = print_lists(kbuf);

        kbuf[read++] = '\0';

        if (copy_to_user(buf,kbuf,read) > 0) return -EFAULT;
        (*off) += read;

        return read;
}

void rb_insert(struct rb_root *root, struct redblack_entry *new)
{
        struct rb_node **link = &root->rb_node, *parent;
        int value = new->val;
        struct redblack_entry *node = NULL;

        /* Go to the bottom of the tree */
        while (*link)
        {
            parent = *link;
            node = rb_entry(parent, struct redblack_entry, node);

            if (node->val > value)
                link = &(*link)->rb_left;
            else
                link = &(*link)->rb_right;
        }

        /* Put the new node there */
        rb_link_node(&new->node, parent, link);
        rb_insert_color(&new->node, &redblack_root);
}

static int store_value(int val)
{
        struct entry *new_item = NULL;
        struct hash_entry *new_item_h = NULL;
        struct redblack_entry *new_redblack = NULL;
        struct radix_entry *radix_entry = NULL;
        struct xarray_entry *xarray_entry = NULL;
        static int key = 0;
        int ret;

        new_item     = kmalloc(sizeof(struct entry),GFP_KERNEL);
        new_item_h   = kmalloc(sizeof(struct hash_entry),GFP_KERNEL);
        new_redblack = kmalloc(sizeof(struct redblack_entry),GFP_KERNEL);
        radix_entry  = kmalloc(sizeof(struct radix_entry),GFP_KERNEL);
        xarray_entry = kmalloc(sizeof(struct xarray_entry),GFP_KERNEL);

        if (unlikely(!new_item || !new_item_h || !new_redblack || !radix_entry))
                return -ENOMEM;

        new_item->val     = val;
        new_item_h->val   = val;
        new_redblack->val = val;
        radix_entry->val  = val;
        xarray_entry->val = val;
        elems++;

        list_add_tail(&new_item->list,&mylist);
        hash_add(hash_table,&new_item_h->hlist,key);
        rb_insert(&redblack_root,new_redblack);
        ret = xa_err(xa_store(&xarray,key,xarray_entry,GFP_KERNEL));

        if (ret){
                printk(KERN_WARNING "Error storing xa value.");
                return -ENOMEM;
        }

        //ret = radix_tree_preload(GFP_KERNEL);

        if (ret == 0){
                radix_tree_insert(&radix_tree,key,radix_entry);
        } else {
                printk(KERN_WARNING "Couldn't preload radix tree.");
        }

        key++;
        //radix_tree_preload_end(); /* Resume preemption */ 

        return 0;
}

static inline void destroy_hash(void)
{
        struct hash_entry* cur_hash_node = NULL;
        struct hlist_node* tmp;
        int key = 0;

        hash_for_each_safe(hash_table,key,tmp,cur_hash_node,hlist) {
                hash_del(&cur_hash_node->hlist);
                kfree(cur_hash_node);
        }
}

static inline void destroy_red_black(void)
{
        struct rb_node *redblack_node    = NULL, *aux;
        struct redblack_entry *rb_entry  = NULL;

        if (!RB_EMPTY_ROOT(&redblack_root)) {
                redblack_node = rb_first(&redblack_root);
        }

        while (redblack_node){
                rb_entry = rb_entry(redblack_node,struct redblack_entry,node);
                aux = rb_next(redblack_node);
                rb_erase(redblack_node,&redblack_root);
                kfree(redblack_node);
                redblack_node = aux;
        }
}

static inline void destroy_linked_list_and_free(void)
{
        struct list_head* cur_node = NULL;
        struct list_head* aux      = NULL;
        struct entry* item         = NULL;

        list_for_each_safe(cur_node, aux, &mylist) {

                item = list_entry(cur_node, struct entry, list);
                list_del(&item->list);
                kfree(item);
        }
}

static inline void destroy_radix(void){

        int key = 0;
        struct radix_entry *radix_entry = NULL;

        do {
                radix_entry = radix_tree_delete(&radix_tree,key++);
                kfree(radix_entry);
        } while (radix_entry && key < elems);
}

static inline void destroy_xarray(void){

        int key = 0;
        struct xarray_entry *xarray_entry = NULL;

        do {
                xarray_entry = xa_erase(&xarray,key++);
                kfree(xarray_entry);
        } while (xarray_entry && key < elems);
}

static int parse_params(void)
{
        int val, err = 0;
        char *p, *orig, *params;

        params = kstrdup(int_str, GFP_KERNEL);
        if (!params)
                return -ENOMEM;

        orig = params;

        while ((p = strsep(&params, ",")) != NULL) {

                if (!*p) continue;

                err = kstrtoint(p, 0, &val);
                if (err) break;

                err = store_value(val);
                if (err) break;
        }

        kfree(orig);
        return err;
}

static void cleanup(void)
{
        printk(KERN_INFO "\nCleaning up...\n");

        destroy_linked_list_and_free();
        destroy_hash();
        destroy_red_black();
        destroy_radix();
        destroy_xarray();
}

const struct proc_ops fops = {
        .proc_read    = proj2_read,
};

static int __init ex5_init(void)
{
        int err = 0;

        proc_proj2 = proc_create("datas", 0666, NULL, &fops); //Create proc entry modconfig

        if (proc_proj2 == NULL) {
                printk(KERN_INFO "Could not create /proc entry\n");
                return -1;
        }

        if (!int_str) {
                printk(KERN_INFO "Missing \'int_str\' parameter, exiting\n");
                return -1;
        }

        hash_init(hash_table);

        err = parse_params();

        return err;
}

static void __exit ex5_exit(void)
{
        remove_proc_entry("datas", NULL);

        cleanup();

        return;
}

module_init(ex5_init);
module_exit(ex5_exit);
