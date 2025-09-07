#ifndef RBTREE_H
#define RBTREE_H

#include "common.h"

enum color_t { RED, BLACK };

// Một nút trong RBT lưu trỏ tới tiến trình
struct rb_node_t {
    struct pcb_t *proc;
    enum color_t color;
    struct rb_node_t *left;
    struct rb_node_t *right;
    struct rb_node_t *parent;
};

struct rbtree_t {
    struct rb_node_t *root;
    int size;
};

// Các hàm cơ bản
void rbtree_init(struct rbtree_t *tree);
void insert_node(struct rbtree_t *tree, struct pcb_t *proc);
void delete_node(struct rbtree_t *tree, struct rb_node_t *node);
struct rb_node_t *create_node(struct pcb_t *proc);
struct pcb_t *pick_next_process(struct rbtree_t *tree); // CFS: get min vruntime
int rbtree_empty(struct rbtree_t *tree);

// Helper
void fix_insert(struct rb_node_t **root, struct rb_node_t *node);
void fix_delete(struct rb_node_t **root, struct rb_node_t *node);
void rotate_left(struct rb_node_t **root, struct rb_node_t *x);
void rotate_right(struct rb_node_t **root, struct rb_node_t *x);
struct rb_node_t *tree_minimum(struct rb_node_t *node);
void transplant(struct rb_node_t** root, struct rb_node_t* u, struct rb_node_t* v);

// Traverse
void inorder_traversal(struct rb_node_t *root);
void destroy_rbtree(struct rbtree_t *tree);
void destroy_subtree(struct rb_node_t *node);
struct pcb_t *find_pcb_by_path(struct rb_node_t *node, char *path, struct pcb_t *caller);

#endif