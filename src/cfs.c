#include "cfs.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static struct rbtree_t ready_tree;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;

// Precomputed weight lookup table for niceness -20 to +19
static const uint32_t weight_lookup[40] = {
    88761, 71755, 56483, 46273, 36291, 29154, 23254, 18705, 14949, 11916,
    9548, 7620, 6100, 4904, 3906, 3121, 2501, 1991, 1586, 1277,
    1024, 820, 655, 526, 423, 335, 272, 215, 172, 137,
    110, 87, 70, 56, 45, 36, 29, 23, 18, 15
};

uint32_t calc_weight(int8_t niceness) {
    return weight_lookup[niceness + 20]; // niceness ∈ [-20, 19]
}

uint32_t calculate_timeslice(struct pcb_t *proc, uint32_t total_weight, uint32_t latency) {
    return (uint64_t)proc->weight * latency / total_weight;
}

void remove_from_running_list(struct pcb_t *proc) {
    remove_from_queue(&running_list, proc);
}

void remove_from_ready_tree(struct pcb_t *proc) {
    pthread_mutex_lock(&queue_lock);
    delete_node(&ready_tree, proc->rb_node);
    pthread_mutex_unlock(&queue_lock);
}

int queue_empty(void) {
    return rbtree_empty(&ready_tree);
}

void init_scheduler(void) {
    pthread_mutex_init(&queue_lock, NULL);
    rbtree_init(&ready_tree);
    running_list.size = 0;
}

void finish_scheduler(void) {
    destroy_rbtree(&ready_tree);
}

struct pcb_t *get_proc(void) {
    pthread_mutex_lock(&queue_lock);
    struct pcb_t *proc = pick_next_process(&ready_tree);
    if (proc != NULL) {
        enqueue(&running_list, proc);
        delete_node(&ready_tree, proc->rb_node);
    }
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t *proc) {
    proc->running_list = & running_list;
    proc->ready_tree = &ready_tree;
    pthread_mutex_lock(&queue_lock);
    insert_node(&ready_tree, proc);
    remove_from_running_list(proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc) {
    proc->running_list = &running_list;
    proc->ready_tree = &ready_tree;
    pthread_mutex_lock(&queue_lock);
    insert_node(&ready_tree, proc);
    pthread_mutex_unlock(&queue_lock);
}

double peek_vruntime(struct pcb_t *proc) {
    pthread_mutex_lock(&queue_lock);
    insert_node(&ready_tree, proc);
    double vruntime = pick_next_process(&ready_tree)->vruntime;
    delete_node(&ready_tree, proc->rb_node);
    pthread_mutex_unlock(&queue_lock);
    return vruntime;
}
