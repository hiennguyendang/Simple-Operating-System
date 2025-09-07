#include "red_black_tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void rbtree_init(struct rbtree_t *tree) {
    tree->root = NULL;
    tree->size = 0;
}

struct rb_node_t *create_node(struct pcb_t *proc) {
    struct rb_node_t *node = (struct rb_node_t *)malloc(sizeof(struct rb_node_t));
    node->proc = proc;
    node->color = RED;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    proc->rb_node = node; // Link the process to its node
    return node;
}

// void rotate_left(struct rbtree_t *tree, struct rb_node_t *x) {
//     struct rb_node_t *y = x->right;
//     x->right = y->left;
//     if (y->left) y->left->parent = x;
//     y->parent = x->parent;
//     if (!x->parent)
//         tree->root = y;
//     else if (x == x->parent->left)
//         x->parent->left = y;
//     else
//         x->parent->right = y;
//     y->left = x;
//     x->parent = y;
// }

void rotate_left(struct rb_node_t** root, struct rb_node_t* x) {
    struct rb_node_t* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

// void rotate_right(struct rbtree_t *tree, struct rb_node_t *x) {
//     struct rb_node_t *y = x->left;
//     x->left = y->right;
//     if (y->right) y->right->parent = x;
//     y->parent = x->parent;
//     if (!x->parent)
//         tree->root = y;
//     else if (x == x->parent->right)
//         x->parent->right = y;
//     else
//         x->parent->left = y;
//     y->right = x;
//     x->parent = y;
// }

void rotate_right(struct rb_node_t** root, struct rb_node_t* x) {
    struct rb_node_t* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

// void fix_insert(struct rbtree_t *tree, struct rb_node_t *node) {
//     while (node != tree->root && node->parent->color == RED) {
//         struct rb_node_t *p = node->parent;
//         struct rb_node_t *g = p->parent;
//         if (p == g->left) {
//             struct rb_node_t *u = g->right;
//             if (u && u->color == RED) {
//                 p->color = BLACK;
//                 u->color = BLACK;
//                 g->color = RED;
//                 node = g;
//             } else {
//                 if (node == p->right) {
//                     node = p;
//                     rotate_left(tree, node);
//                 }
//                 node->parent->color = BLACK;
//                 node->parent->parent->color = RED;
//                 rotate_right(tree, node->parent->parent);
//             }
//         } else {
//             struct rb_node_t *u = g->left;
//             if (u && u->color == RED) {
//                 p->color = BLACK;
//                 u->color = BLACK;
//                 g->color = RED;
//                 node = g;
//             } else {
//                 if (node == p->left) {
//                     node = p;
//                     rotate_right(tree, node);
//                 }
//                 node->parent->color = BLACK;
//                 node->parent->parent->color = RED;
//                 rotate_left(tree, node->parent->parent);
//             }
//         }
//     }
//     tree->root->color = BLACK;
// }

void fix_insert(struct rb_node_t** root, struct rb_node_t* z) {
    while (z->parent && z->parent->color == RED) {
        struct rb_node_t* grandparent = z->parent->parent;
        if (z->parent == grandparent->left) {
            struct rb_node_t* y = grandparent->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                grandparent->color = RED;
                z = grandparent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotate_left(root, z);
                }
                z->parent->color = BLACK;
                grandparent->color = RED;
                rotate_right(root, grandparent);
            }
        } else {
            struct rb_node_t* y = grandparent->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                grandparent->color = RED;
                z = grandparent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(root, z);
                }
                z->parent->color = BLACK;
                grandparent->color = RED;
                rotate_left(root, grandparent);
            }
        }
    }
    (*root)->color = BLACK;
}

void insert_node(struct rbtree_t *tree, struct pcb_t *proc) {
    struct rb_node_t *node = create_node(proc);
    struct rb_node_t *y = NULL;
    struct rb_node_t *x = tree->root;
    while (x) {
        y = x;
        if (proc->vruntime < x->proc->vruntime)
            x = x->left;
        else
            x = x->right;
    }
    node->parent = y;
    if (!y)
        tree->root = node;
    else if (proc->vruntime < y->proc->vruntime)
        y->left = node;
    else
        y->right = node;
    
    fix_insert(&tree->root, node);
    tree->size++;
}

// void fix_delete(struct rbtree_t *tree, struct rb_node_t *x) {
//     while (x != tree->root && x->color == BLACK) {
//         struct rb_node_t *w;
//         if (x == x->parent->left) {
//             w = x->parent->right;
//             if (w->color == RED) {
//                 w->color = BLACK;
//                 x->parent->color = RED;
//                 rotate_left(tree, x->parent);
//                 w = x->parent->right;
//             }
//             if ((!w->left || w->left->color == BLACK )&& (!w->right || w->right->color == BLACK)) {
//                 w->color = RED;
//                 x = x->parent;
//             } else {
//                 if (!w->right || w->right->color == BLACK) {
//                     w->left->color = BLACK;
//                     w->color = RED;
//                     rotate_right(tree, w);
//                     w = x->parent->right;
//                 }
//                 w->color = x->parent->color;
//                 x->parent->color = BLACK;
//                 w->right->color = BLACK;
//                 rotate_left(tree, x->parent);
//                 x = tree->root;
//             }
//         } else {
//             w = x->parent->left;
//             if (w->color == RED) {
//                 w->color = BLACK;
//                 x->parent->color = RED;
//                 rotate_right(tree, x->parent);
//                 w = x->parent->left;
//             }
//             if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
//                 w->color = RED;
//                 x = x->parent;
//             } else {
//                 if (!w->left || w->left->color == BLACK) {
//                     w->right->color = BLACK;
//                     w->color = RED;
//                     rotate_left(tree, w);
//                     w = x->parent->left;
//                 }
//                 w->color = x->parent->color;
//                 x->parent->color = BLACK;
//                 w->left->color = BLACK;
//                 rotate_right(tree, x->parent);
//                 x = tree->root;
//             }
//         }
//     }
//     x->color = BLACK;
// }

void fix_delete(struct rb_node_t** root, struct rb_node_t* x) {
    while (x != *root && (!x || x->color == BLACK)) {
        if (x == x->parent->left) {
            struct rb_node_t* w = x->parent->right;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_left(root, x->parent);
                w = x->parent->right;
            }
            if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->right || w->right->color == BLACK) {
                    if (w->left) w->left->color = BLACK;
                    w->color = RED;
                    rotate_right(root, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->right) w->right->color = BLACK;
                rotate_left(root, x->parent);
                x = *root;
            }
        } else {
            struct rb_node_t* w = x->parent->left;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_right(root, x->parent);
                w = x->parent->left;
            }
            if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->left || w->left->color == BLACK) {
                    if (w->right) w->right->color = BLACK;
                    w->color = RED;
                    rotate_left(root, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->left) w->left->color = BLACK;
                rotate_right(root, x->parent);
                x = *root;
            }
        }
    }
    if (x) x->color = BLACK;
}

// void delete_node(struct rbtree_t *tree, struct rb_node_t *node) {
//     node->proc->rb_node = NULL; // Unlink the process from its node
//     struct rb_node_t *y = node;
//     struct rb_node_t *x;
//     enum color_t y_original_color = y->color;
//     if (!node->left) {
//         x = node->right;
//         if (x) x->parent = node->parent;
//         if (!node->parent)
//             tree->root = x;
//         else if (node == node->parent->left)
//             node->parent->left = x;
//         else
//             node->parent->right = x;
//     } else if (!node->right) {
//         x = node->left;
//         if (x) x->parent = node->parent;
//         if (!node->parent)
//             tree->root = x;
//         else if (node == node->parent->left)
//             node->parent->left = x;
//         else
//             node->parent->right = x;
//     } else {
//         y = tree_minimum(node->right);
//         y_original_color = y->color;
//         x = y->right;
//         if (y == node->right) {
//             if (x) x->parent = y;
//         } else {
//             if (x) x->parent = y->parent;
//             y->parent->left = x;
//             y->right = node->right;
//             if (node->right) node->right->parent = y;
//         }
//         if (!node->parent)
//             tree->root = y;
//         else if (node == node->parent->left)
//             node->parent->left = y;
//         else
//             node->parent->right = y;
//         y->left = node->left;
//         if (node->left) node->left->parent = y;
//         y->color = node->color;
//     }
//     free(node);
//     tree->size--;
//     if (y_original_color == BLACK && x) fix_delete(tree, x);
// }

void delete_node(struct rbtree_t *tree, struct rb_node_t *node) {
    if (!node) return; // Check for NULL node
    node->proc->rb_node = NULL; // Unlink the process from its node
    struct rb_node_t* y = node;
    enum color_t y_original_color = y->color;
    struct rb_node_t* x;
    if (!node->left) {
        x = node->right;
        transplant(&tree->root, node, node->right);
    } else if (!node->right) {
        x = node->left;
        transplant(&tree->root, node, node->left);
    } else {
        y = tree_minimum(node->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == node && x) {
            x->parent = y;
        } else {
            transplant(&tree->root, y, y->right);
            y->right = node->right;
            if (y->right) y->right->parent = y;
        }
        transplant(&tree->root, node, y);
        y->left = node->left;
        if (y->left) y->left->parent = y;
        y->color = node->color;
    }
    free(node);
    tree->size--;
    if (y_original_color == BLACK && x) fix_delete(&tree->root, x);
}

void transplant(struct rb_node_t** root, struct rb_node_t* u, struct rb_node_t* v) {
    if (!u->parent) *root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

struct rb_node_t *tree_minimum(struct rb_node_t *node) {
    if (!node) return NULL;
    while (node->left)
        node = node->left;
    return node;
}

struct pcb_t *pick_next_process(struct rbtree_t *tree) {
    if (!tree->root) return NULL;
    struct rb_node_t *min = tree_minimum(tree->root);
    return min ? min->proc : NULL;
}

int rbtree_empty(struct rbtree_t *tree) {
    return tree->root == NULL;
}

void destroy_subtree(struct rb_node_t *node) {
    if (!node) return;
    destroy_subtree(node->left);
    destroy_subtree(node->right);
    free(node);
}

void destroy_rbtree(struct rbtree_t *tree) {
    destroy_subtree(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

void inorder_traversal(struct rb_node_t *root) {
    if (!root) return;
    inorder_traversal(root->left);
    printf("PID: %d, vruntime: %f\n", root->proc->pid, root->proc->vruntime);
    inorder_traversal(root->right);
}

struct pcb_t *find_pcb_by_path(struct rb_node_t *node, char *path, struct pcb_t *caller) {
    if (!node) return NULL;
    if (strcmp(node->proc->path, path) == 0 && node->proc != caller && node->proc->pid != 0) {
        return node->proc;
    }
    struct pcb_t *left_proc = find_pcb_by_path(node->left, path, caller);
    if (left_proc) return left_proc;
    struct pcb_t *right_proc = find_pcb_by_path(node->right, path, caller);
    if (right_proc) return right_proc;
    return NULL;
}