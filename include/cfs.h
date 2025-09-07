#ifndef CFS_H
#define CFS_H

#include "common.h"
#include "red_black_tree.h"
#include "queue.h"

#ifndef CFS_SCHED
#define CFS_SCHED
#endif

uint32_t calc_weight(int8_t niceness);
uint32_t calculate_timeslice(struct pcb_t *proc, uint32_t total_weight, uint32_t latency);

void init_scheduler(void);
void finish_scheduler(void);

int queue_empty(void);
struct pcb_t *get_proc(void);
void add_proc(struct pcb_t *proc);
void put_proc(struct pcb_t *proc);
double peek_vruntime(struct pcb_t *proc);
void remove_from_running_list(struct pcb_t *proc);
void remove_from_ready_tree(struct pcb_t *proc);

#endif
