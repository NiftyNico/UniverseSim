/*
 * =============================================================================
 *
 *       Filename:  pthread-barrier.h
 *
 *    Description:  pthread barrier implementation.
 *
 *        Created:  12/26/2012 11:08:06 AM
 *
 *         Author:  Fu Haiping (forhappy), haipingf@gmail.com
 *        Company:  ICT ( Institute Of Computing Technology, CAS )
 *
 * =============================================================================
 */
#ifndef _PTHREAD_BARRIER_H_
#define _PTHREAD_BARRIER_H_

#include <pthread.h>

 #define PTHREAD_BARRIER_SERIAL_THREAD 1

typedef struct barrier_s_ pthread_barrier_t;

struct barrier_s_ {
	unsigned count;
	unsigned total;
	pthread_mutex_t m;
	pthread_cond_t cv;
};

#define BARRIER_FLAG (1UL<<31)

extern void pthread_barrier_init(pthread_barrier_t *b, void *attr, unsigned count);

extern int pthread_barrier_wait(pthread_barrier_t *b);

extern void pthread_barrier_destroy(pthread_barrier_t *b);

#endif /* _PTHREAD_BARRIER_H_ */
