//reference b10902062

#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args) {
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list)
    {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args) {
    struct thread *th = NULL;
    struct thread *run = NULL;
    struct thread *miss = NULL;
    struct release_queue_entry *release = NULL;
    struct threads_sched_result r;

    list_for_each_entry(th, args.run_queue, thread_list)
    {// if miss
        if (th->current_deadline <= args.current_time) {
            if (miss == NULL)miss = th;
            else if (th->ID < miss->ID)miss = th;
        }
    }

    if (miss != NULL) {// if miss == true, return
        r.allocated_time = 0;
        r.scheduled_thread_list_member = &miss->thread_list;
        return r;
    }

    th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list)
    {//set running thrd
        if (run == NULL) run = th;
        else if (th->current_deadline == run->current_deadline && th->ID < run->ID) { // there is ealier ddl
            run = th;
        } else if (th->current_deadline < run->current_deadline) {
            run = th;
        }
    }

    if (run != NULL) { // something is running
        r.scheduled_thread_list_member = &run->thread_list;
        int ticks = args.current_time + 1;
        while (ticks < args.current_time + run->remaining_time) {//find if running time hits a ddl
            if (ticks == run->current_deadline) {//allocate remaining finish time
                r.allocated_time = run->current_deadline - args.current_time;
                return r;
            }
            release = NULL;
            int preempted = 0;
            struct thread *earlier = run;
            int mn = run->current_deadline;
            list_for_each_entry(release, args.release_queue, thread_list)
            {// if there's a preempted thrd
                if (release->thrd->current_deadline == ticks) {
                    if (ticks + release->thrd->period < mn) {
                        earlier = release->thrd;
                        preempted = 1;
                        mn = ticks + release->thrd->period;
                    } else if (ticks + release->thrd->period == mn && release->thrd->ID < earlier->ID) {
                        earlier = release->thrd;
                        preempted = 1;
                        mn = ticks + release->thrd->period;
                    }
                }
            }
            if (preempted) {//thread switch
                r.allocated_time = ticks - args.current_time;
                return r;
            }
            ticks++;
        }
        r.allocated_time = run->remaining_time;
        return r;
    } else {
        release = NULL;
        struct thread *first_r = NULL;
        list_for_each_entry(release, args.release_queue, thread_list)
        {
//            printf("release->thrd->current_deadline = %d\n", release->thrd->current_deadline);
            if (first_r == NULL) {// no other release
                first_r = release->thrd;
            } else if (release->thrd->current_deadline < first_r->current_deadline) {
                first_r = release->thrd;
            } else if (release->thrd->current_deadline == first_r->current_deadline &&
                       release->thrd->ID < first_r->ID) {
                first_r = release->thrd;
            }
        }
        r.allocated_time = first_r->current_deadline - args.current_time;
//        printf("r.allocated_time = %d, first_r->current_deadline = %d, args.current_time = %d\n", r.allocated_time, first_r->current_deadline, args.current_time);
        r.scheduled_thread_list_member = args.run_queue;
        return r;
    }
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args) {

    struct thread *th = NULL;
    struct thread *run = NULL;
    struct thread *miss = NULL;
    struct release_queue_entry *release = NULL;
    struct threads_sched_result r;

    list_for_each_entry(th, args.run_queue, thread_list)
    {// if miss
        if (th->current_deadline <= args.current_time) {
            if (miss == NULL)miss = th;
            else if (th->ID < miss->ID)miss = th;
        }
    }

    if (miss != NULL) {// if miss == true, return
        r.allocated_time = 0;
        r.scheduled_thread_list_member = &miss->thread_list;
        return r;
    }
    th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list)
    {
        if (run == NULL)run = th;
        else if (th->period == run->period && th->ID < run->ID)run = th;
        else if (th->period < run->period)run = th;
    }

    if (run != NULL) {
        r.scheduled_thread_list_member = &run->thread_list;
        int ticks = args.current_time + 1;
        while (ticks < args.current_time + run->remaining_time) {
            if (ticks == run->current_deadline) {
                r.allocated_time = run->current_deadline - args.current_time;
                return r;
            }

            release = NULL;
            int preempted = 0;
            int mn = run->period;
            struct thread *min_period = run;
            list_for_each_entry(release, args.release_queue, thread_list)
            {
                if (release->thrd->current_deadline == ticks) {
                    if (release->thrd->period == mn && release->thrd->ID < min_period->ID) {
                        min_period = release->thrd;
                        preempted = 1;
                        mn = release->thrd->period;
                    } else if (release->thrd->period < mn) {
                        min_period = release->thrd;
                        preempted = 1;
                        mn = release->thrd->period;
                    }
                }
            }
            if (preempted) {
                r.allocated_time = ticks - args.current_time;
                return r;
            }
            ticks++;
        }

        r.allocated_time = run->remaining_time;
        return r;
    } else {
        release = NULL;
        struct thread *first_r = NULL;
        list_for_each_entry(release, args.release_queue, thread_list)
        {
            if (first_r == NULL)first_r = release->thrd;
            else if (release->thrd->current_deadline < first_r->current_deadline) {
                first_r = release->thrd;
            } else if (release->thrd->current_deadline == first_r->current_deadline &&
                       release->thrd->ID < first_r->ID) {
                first_r = release->thrd;
            }
        }
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = first_r->current_deadline - args.current_time;
        return r;
    }

}
