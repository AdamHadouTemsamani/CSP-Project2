#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>

// Baton for each child work request
typedef struct {
    uv_work_t req;       // libuv’s work-request handle
    uint32_t *arr;       // the full integer array to sort
    size_t left, right;  // this child’s subarray bounds
    uv_mutex_t *mutex;   // pointer to a shared parent mutex
    int *parent_counter; // pointer to a shared parent counter
    size_t parent_left;  // parent’s left bound (for merge)
    size_t parent_right; // parent’s right bound (for merge)
} sort_baton_t;

// Runs in a thread-pool thread: sort the assigned subarray
static void work_cb(uv_work_t *req) {
    sort_baton_t *baton = req->data;
    fprintf(stderr, "[work_cb] thread %lu sorting [%zu..%zu]\n", (unsigned long)uv_thread_self(), baton->left,
            baton->right);
    merge_sort(baton->arr, baton->left, baton->right);
}

// Runs on the loop thread: track completion and merge when both children done
static void after_cb(uv_work_t *req, int status) {
    sort_baton_t *baton = req->data;
    fprintf(stderr, "[after_cb] thread %lu finished [%zu..%zu]\n", (unsigned long)uv_thread_self(), baton->left,
            baton->right);

    uv_mutex_lock(baton->mutex);
    (*baton->parent_counter)++;
    if (*baton->parent_counter == 2) {
        int mid = baton->parent_left + (baton->parent_right - baton->parent_left) / 2;
        fprintf(stderr, "[merge] merging [%zu..%zu] at mid %d\n", baton->parent_left, baton->parent_right, mid);
        merge(baton->arr, baton->parent_left, mid, baton->parent_right);

        // cleanup parent sync objects
        uv_mutex_unlock(baton->mutex);
        uv_mutex_destroy(baton->mutex);
        free(baton->mutex);
        free(baton->parent_counter);
        return;
    }
    uv_mutex_unlock(baton->mutex);
    free(baton);
}

void merge_sort_uv(uint32_t *arr, size_t left, size_t right, uv_loop_t *loop, uv_mutex_t *unused_mutex) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;

    // Allocate parent sync objects
    uv_mutex_t *parent_mutex = malloc(sizeof(*parent_mutex));
    int *parent_counter = malloc(sizeof(*parent_counter));
    uv_mutex_init(parent_mutex);
    *parent_counter = 0;

    // Left child baton
    sort_baton_t *lb = malloc(sizeof(*lb));
    lb->req.data = lb;
    lb->arr = arr;
    lb->left = left;
    lb->right = mid;
    lb->mutex = parent_mutex;
    lb->parent_counter = parent_counter;
    lb->parent_left = left;
    lb->parent_right = right;
    uv_queue_work(loop, &lb->req, work_cb, after_cb);

    // Right child baton
    sort_baton_t *rb = malloc(sizeof(*rb));
    rb->req.data = rb;
    rb->arr = arr;
    rb->left = mid + 1;
    rb->right = right;
    rb->mutex = parent_mutex;
    rb->parent_counter = parent_counter;
    rb->parent_left = left;
    rb->parent_right = right;
    uv_queue_work(loop, &rb->req, work_cb, after_cb);
}
