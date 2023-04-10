#include <defs.h>
#include <proc.h>
#include <assert.h>
#include <skew_heap.h>
#include <priority_sched.h>

static int
compare_proc(skew_heap_entry_t *_lhs, skew_heap_entry_t *_rhs) {
    struct proc_struct *lhs = she2proc(_lhs, proc_pool_entry);
    struct proc_struct *rhs = she2proc(_rhs, proc_pool_entry);
    if (lhs->labschedule_good > rhs->labschedule_good) {
        return -1;
    }
    else if (lhs->labschedule_good == rhs->labschedule_good) {
        if (lhs->proc_pool_entry_time < rhs->proc_pool_entry_time) {
            return -1;
        }
        else if (lhs->proc_pool_entry_time == rhs->proc_pool_entry_time) {
            return 0;
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }
}

static void
priority_init(struct run_queue *rq) {
    rq->labschedule_run_pool = NULL;
    rq->proc_num = 0;
}

static size_t entry_time = 0;

static void
priority_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    proc->proc_pool_entry_time = ++entry_time;
    rq->labschedule_run_pool = skew_heap_insert(rq->labschedule_run_pool, &(proc->proc_pool_entry), (compare_f)compare_proc);
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;
    }
    proc->rq = rq;
    rq->proc_num ++;
}

static void
priority_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    assert((proc->proc_pool_entry_time != 0) && proc->rq == rq);
    proc->proc_pool_entry_time = 0;
    rq->labschedule_run_pool = skew_heap_remove(rq->labschedule_run_pool, &(proc->proc_pool_entry), (compare_f)compare_proc);
    rq->proc_num --;
}

static struct proc_struct *
priority_pick_next(struct run_queue *rq) {
    if (rq->labschedule_run_pool != NULL)
        return she2proc(rq->labschedule_run_pool, proc_pool_entry);
    return NULL;
}

static void
priority_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}

struct sched_class priority_sched_class = {
    .name = "priority_scheduler",
    .init = priority_init,
    .enqueue = priority_enqueue,
    .dequeue = priority_dequeue,
    .pick_next = priority_pick_next,
    .proc_tick = priority_proc_tick,
};

