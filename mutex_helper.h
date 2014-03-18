#ifndef MUTEX_HELPER_H
#define MUTEX_HELPER_H

struct Mutex_map_elem* search_mutex_map(pthread_mutex_t *mutex);
void add_to_mutex_map(pthread_mutex_t *mutex,pthread_t holder);
void add_waiter(struct Mutex_map_elem *mmap_elem , pthread_t waiter);
void release_waiters(struct Mutex_map_elem *mmap_elem);
void remove_from_waiters(struct Mutex_map_elem *mut,pthread_t thread);
void remove_from_mutex_map(struct Mutex_map_elem *);
#endif
