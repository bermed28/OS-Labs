//#include "proc.h"
void enqueue(struct proc *rp	/* this process is now runnable */) {
    /********** Your enqueue modification should be below ************/
    /* Now add the process to the queue. */
    /*If queue is empty, add just the single element to the queue, regardless of deadline*/
    if (!rdy_head[q]) {
        rdy_head[q] = rdy_tail[q] = rp;
        rp->p_nextready = NULL;
    } else if(rp->deadline == 0) {
        /*If deadline is 0, enqueue normally to end of queue*/
        rdy_tail[q]->p_nextready = rp;
        rdy_tail[q] = rp;
        rp->p_nextready = NULL;
    } else{
        /* If deadline is non-zero and rp has smallest deadline priorty, add to front of queue */
        if (rdy_head[q]->deadline > rp->deadline || rdy_head[q]->deadline == 0) {
            rp->p_nextready = rdy_head[q];
            rdy_head[q] = rp;
        } else {
            /* If deadline priority is non-zero, prune search the correct spot for rp based on its deadline priority*/
            struct proc *current_node = rdy_head[q];
            struct proc *next_node = current_node->p_nextready;
            while (next_node != rdy_tail[q]->p_nextready && (next_node->deadline < rp->deadline && next_node->deadline != 0)) {
                current_node = next_node;
                next_node = next_node->p_nextready;
            }
            /* If we reached the end of the queue, it means this process has the highest deadline, so it's added at the end of the queue*/
            if(next_node == rdy_tail[q]->p_nextready){
                rdy_tail[q]->p_nextready = rp;
                rdy_tail[q] = rp;
                rp->p_nextready = NULL;
            } else {
                /* If not, we add it in between the processes it needs to go */
                rp->p_nextready = next_node;
                current_node->p_nextready = rp;
            }
        }
    }
    /********* Your enqueue modification should be above ************/
}
