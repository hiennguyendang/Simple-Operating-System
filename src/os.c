
#include "cpu.h"
#include "timer.h"
#include "cfs.h"
#include "loader.h"
#include "mm.h"
#include "libmem.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int time_slot;
static int num_cpus;
static int done = 0;

#ifdef MM_PAGING
static int memramsz;
static int memswpsz[PAGING_MAX_MMSWP];

struct mmpaging_ld_args {
	/* A dispatched argument struct to compact many-fields passing to loader */
	int vmemsz;
	struct memphy_struct *mram;
	struct memphy_struct **mswp;
	struct memphy_struct *active_mswp;
	int active_mswp_id;
	struct timer_id_t  *timer_id;
};
#endif

static struct ld_args{
	char ** path;
	unsigned long * start_time;
#ifdef CFS_SCHED
	int * prio;
	uint32_t latency;
	uint32_t total_weight;
#endif
} ld_processes;
int num_processes;
uint32_t total_wait_time = 0;

struct cpu_args {
	struct timer_id_t * timer_id;
	int id;
};


static void * cpu_routine(void * args) {
	struct timer_id_t * timer_id = ((struct cpu_args*)args)->timer_id;
	int id = ((struct cpu_args*)args)->id;

	struct pcb_t * proc = NULL;

	while (1) {
		uint64_t time = current_time();
		if (proc == NULL) {
			proc = get_proc(); // lấy process có vruntime thấp nhất
			if (proc == NULL && !done) {
				next_slot(timer_id);
				continue;
			}
		}
		if (proc && proc->pc == proc->code->size) {
			printf("\tCPU %d: Processed %2d has finished\n", id ,proc->pid);
			total_wait_time += current_time() - proc->arrival_time - proc->time_executed + proc->time_temp;
			remove_from_running_list(proc);
			free_pbc_mem(proc);
			free(proc);
			proc = get_proc();
		}

		if (proc == NULL && done) {
			printf("\tCPU %d stopped\n", id);
			break;
		}
		else if (proc == NULL) {
			next_slot(timer_id);
			continue;
		}else if (proc->time_run == 0)
			printf("\tCPU %d: Dispatched process %2d\n", id, proc->pid);

		/* Chạy 1 tick */
		printf("........................\n");
		printf("Running on CPU %d: %2d\n", id, proc->pid);
		printf("........................\n");
		if (proc->time_slot != time) {
			proc->time_slot = time;
			proc->time_executed++;
		}
		run(proc);
		proc->time_run++;

		/* Cập nhật vruntime */
		proc->vruntime += 1.0 / proc->weight;

		/* Nếu chạy hết time_slice hoặc bị process khác có vruntime thấp hơn đẩy ra */
		double vruntime = peek_vruntime(proc);
		if (proc->time_run >= (int)proc->time_slice || vruntime < proc->vruntime) {
			printf("\tCPU %d: Put process %2d to run tree\n", id, proc->pid);
			put_proc(proc);
			proc->time_run = 0;
			proc = NULL;
		}
		next_slot(timer_id);
	}

	detach_event(timer_id);
	pthread_exit(NULL);
}


static void * ld_routine(void * args) {  //if procs have same arrival time?
#ifdef MM_PAGING
	struct memphy_struct* mram = ((struct mmpaging_ld_args *)args)->mram;
	struct memphy_struct** mswp = ((struct mmpaging_ld_args *)args)->mswp;
	struct memphy_struct* active_mswp = ((struct mmpaging_ld_args *)args)->active_mswp;
	uint32_t active_mswp_id = ((struct mmpaging_ld_args *)args)->active_mswp_id;
	struct timer_id_t * timer_id = ((struct mmpaging_ld_args *)args)->timer_id;
#else
	struct timer_id_t * timer_id = (struct timer_id_t*)args;
#endif
	int i = 0;
	printf("ld_routine\n");
	while (i < num_processes) {
		struct pcb_t * proc = load(ld_processes.path[i]);
#ifdef CFS_SCHED
		proc->prio = ld_processes.prio[i];
		proc->weight = calc_weight(proc->prio);
		proc->vruntime = 0;
		proc->time_run = 0;
		proc->time_slice = calculate_timeslice(proc, ld_processes.total_weight, ld_processes.latency);
		proc->time_slot = -1;
		proc->time_executed = 0;
		proc->time_temp = 0;
#endif
		while (current_time() < ld_processes.start_time[i]) {
			next_slot(timer_id);
		}
#ifdef MM_PAGING
		proc->mm = malloc(sizeof(struct mm_struct));
		init_mm(proc->mm, proc);
		proc->mram = mram;
		proc->mswp = mswp;
		proc->active_mswp = active_mswp;
		proc->active_mswp_id = active_mswp_id;
#endif
		printf("\tLoaded a process at %s, PID: %d Niceness: %d\n", ld_processes.path[i], proc->pid, ld_processes.prio[i]);
		add_proc(proc);
		free(ld_processes.path[i]);
		proc->arrival_time = ld_processes.start_time[i];
		i++;
		next_slot(timer_id);
	}
	free(ld_processes.path);
	free(ld_processes.start_time);
	done = 1;
	detach_event(timer_id);
	pthread_exit(NULL);
}

static void read_config(const char * path) {
	FILE * file;
	if ((file = fopen(path, "r")) == NULL) {
		printf("Cannot find configure file at %s\n", path);
		exit(1);
	}
	fscanf(file, "%d %d %d\n", &time_slot, &num_cpus, &num_processes);
	ld_processes.path = (char**)malloc(sizeof(char*) * num_processes);
	ld_processes.start_time = (unsigned long*)malloc(sizeof(unsigned long) * num_processes);
	ld_processes.latency = time_slot;
#ifdef MM_PAGING
	int sit;
	/* Read input config of memory size: MEMRAM and upto 4 MEMSWP (mem swap)
	   Format: (size=0 result non-used memswap, must have RAM and at least 1 SWAP)
	            MEM_RAM_SZ MEM_SWP0_SZ MEM_SWP1_SZ MEM_SWP2_SZ MEM_SWP3_SZ
	*/
	fscanf(file, "%d\n", &memramsz);
	for(sit = 0; sit < PAGING_MAX_MMSWP; sit++)
		fscanf(file, "%d", &(memswpsz[sit]));
    fscanf(file, "\n"); /* Final character */
#endif

#ifdef CFS_SCHED
	ld_processes.prio = (int*)malloc(sizeof(int) * num_processes);
	ld_processes.total_weight = 0;
#endif
	int i;
	for (i = 0; i < num_processes; i++) {
		ld_processes.path[i] = (char*)malloc(sizeof(char) * 100);
		ld_processes.path[i][0] = '\0';
		strcat(ld_processes.path[i], "input/proc/");
		char proc[100];
#ifdef CFS_SCHED
		fscanf(file, "%lu %s %u\n", &ld_processes.start_time[i], proc, &ld_processes.prio[i]);
		ld_processes.total_weight += calc_weight(ld_processes.prio[i]);
#else
		fscanf(file, "%lu %s\n", &ld_processes.start_time[i], proc);
#endif
		strcat(ld_processes.path[i], proc);
	}
}

int main(int argc, char * argv[]) {
	/* Read config */
	if (argc != 2) {
		printf("Usage: os [path to configure file]\n");
		return 1;
	}
	char path[100];
	path[0] = '\0';
	strcat(path, "input/");
	strcat(path, argv[1]);
	read_config(path);

	pthread_t * cpu = (pthread_t*)malloc(num_cpus * sizeof(pthread_t));
	struct cpu_args * args = (struct cpu_args*)malloc(sizeof(struct cpu_args) * num_cpus);
	pthread_t ld;
	
	/* Init timer */
	int i;
	for (i = 0; i < num_cpus; i++) {
		args[i].timer_id = attach_event();
		args[i].id = i;
	}
	struct timer_id_t * ld_event = attach_event();
	start_timer();

#ifdef MM_PAGING
	/* Init all MEMPHY include 1 MEMRAM and n of MEMSWP */
	int rdmflag = 1; /* By default memphy is RANDOM ACCESS MEMORY */
	struct memphy_struct mram;
	struct memphy_struct mswp[PAGING_MAX_MMSWP];
	/* Create MEM RAM */
	init_memphy(&mram, memramsz, rdmflag);
    /* Create all MEM SWAP */ 
	int sit;
	for(sit = 0; sit < PAGING_MAX_MMSWP; sit++)
	    init_memphy(&mswp[sit], memswpsz[sit], rdmflag);
	/* In Paging mode, it needs passing the system mem to each PCB through loader*/
	struct mmpaging_ld_args *mm_ld_args = malloc(sizeof(struct mmpaging_ld_args));
	mm_ld_args->timer_id = ld_event;
	mm_ld_args->mram = (struct memphy_struct *) &mram;
	mm_ld_args->mswp = (struct memphy_struct**) &mswp;
	mm_ld_args->active_mswp = (struct memphy_struct *) &mswp[0];
    mm_ld_args->active_mswp_id = 0;
#endif
	/* Init scheduler */
	init_scheduler();
	/* Run CPU and loader */
#ifdef MM_PAGING
	pthread_create(&ld, NULL, ld_routine, (void*)mm_ld_args);
#else
	pthread_create(&ld, NULL, ld_routine, (void*)ld_event);
#endif
	for (i = 0; i < num_cpus; i++)
		pthread_create(&cpu[i], NULL, cpu_routine, (void*)&args[i]);
	/* Wait for CPU and loader finishing */
	for (i = 0; i < num_cpus; i++)
		pthread_join(cpu[i], NULL);
	pthread_join(ld, NULL);
	/* Stop timer */

	printf("==================================================================\n");
	double avg_wait_time = 1.0 * total_wait_time / num_processes;
	printf("Average wait time: %lf\n", avg_wait_time);
	printf("Total wait time: %d\n", total_wait_time);
	printf("==================================================================\n");

	stop_timer();
	return 0;
}



