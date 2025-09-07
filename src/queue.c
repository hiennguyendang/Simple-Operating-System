#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q){
        if (q == NULL) return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc) {
        if (q == NULL || proc == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) return;
        for(int i = 0;i<q->size;i++){  //check for duplicate
                if(q->proc[i]->pid == proc->pid) return;
        }
        // Add process to the endd of the queue
        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t *dequeue(struct queue_t *q) {
        if (q == NULL || q->size == 0) return NULL;
        // Retrieve thee process with the highest priority
        struct pcb_t *selected_proc = q->proc[0];
        // Shift elements after max_index to the left to fill the gap
        for (int i = 0; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return selected_proc;
}

void remove_from_queue(struct queue_t *q, struct pcb_t *proc) {
        if (q == NULL || proc == NULL) return;
        for (int i = 0; i < q->size; i++) {
                if (q->proc[i]->pid == proc->pid) {
                        // Shift elements after the found process to the left
                        for (int j = i; j < q->size - 1; j++) {
                                q->proc[j] = q->proc[j + 1];
                        }
                        q->proc[q->size - 1] = NULL; // Clear the last element
                        q->size--;
                        break;
                }
        }
}