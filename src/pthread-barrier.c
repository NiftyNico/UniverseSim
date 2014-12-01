/*
 * =============================================================================
 *
 *       Filename:  pthread-barrier.c
 *
 *    Description:  pthread barrier implementation.
 *
 *        Created:  12/26/2012 11:08:14 AM
 *
 *         Author:  Fu Haiping (forhappy), haipingf@gmail.com
 *        Company:  ICT ( Institute Of Computing Technology, CAS )
 *
 * =============================================================================
 */

#include "pthread-barrier.h"

void pthread_barrier_destroy(pthread_barrier_t *b)
{
	pthread_mutex_lock(&b->m);

	while (b->total > BARRIER_FLAG)
	{
		/* 等待所有线程退出 barrier. */
		pthread_cond_wait(&b->cv, &b->m);
	}

	pthread_mutex_unlock(&b->m);

	pthread_cond_destroy(&b->cv);
	pthread_mutex_destroy(&b->m);
}

void pthread_barrier_init(pthread_barrier_t *b, void *attr, unsigned count)
{
	pthread_mutex_init(&b->m, NULL);
	pthread_cond_init(&b->cv, NULL);
	b->count = count;
	b->total = BARRIER_FLAG;
}

int pthread_barrier_wait(pthread_barrier_t *b)
{
	pthread_mutex_lock(&b->m);

	while (b->total > BARRIER_FLAG)
	{
		pthread_cond_wait(&b->cv, &b->m);
	}

	/* 是否为第一个到达 barrier 的线程? */
	if (b->total == BARRIER_FLAG) b->total = 0;

	b->total++;

	if (b->total == b->count)
	{
		b->total += BARRIER_FLAG - 1;
		pthread_cond_broadcast(&b->cv);

		pthread_mutex_unlock(&b->m);

		return PTHREAD_BARRIER_SERIAL_THREAD;
	}
	else
	{
		while (b->total < BARRIER_FLAG)
		{
			pthread_cond_wait(&b->cv, &b->m);
		}

		b->total--;

		/* 唤醒所有进入 barrier 的线程. */
		if (b->total == BARRIER_FLAG) pthread_cond_broadcast(&b->cv);

		pthread_mutex_unlock(&b->m);

		return 0;
	}
}
