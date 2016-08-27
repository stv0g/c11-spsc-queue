/** Lock-free Single-Producer Single-consumer (SPSC) queue.
 *
 * @author Steffen Vogel <post@steffenvogel.de>
 * @copyright 2016 Steffen Vogel
 * @license BSD 2-Clause License
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <errno.h>

/** Cache line size on modern x86 processors (in bytes) */
#define CACHE_LINE_SIZE	64

struct queue {
	size_t capacity;	/**< Total number of available pointers in queue::array */

	/* Consumer part */
	atomic_int tail;	/**< Tail pointer of queue*/
	
	char _pad[CACHE_LINE_SIZE];
	
	/* Producer part */
	atomic_int head;	/**< */
	
	void *pointers[];	/**< Circular buffer. */	//--? confirm how it is different from void * pointers;, allocate whole mem to queue
};

/** Initiliaze a new queue and allocate memory. */
int queue_init(struct queue *q, size_t len);

/** Release memory of queue. */
void queue_destroy(struct queue *q);

/** Enqueue up to \p cnt elements from \p ptrs[] at the queue tail pointed by \p tail.
 *
 * It may happen that the queue is (nearly) full and there is no more
 * space to enqueue more elments.
 * In this case a call to this function will return a value which is smaller than \p cnt
 * or even zero if the queue was already full.
 *
 * @param q A pointer to the queue datastructure.
 * @param[in] ptrs An array of void-pointers which should be enqueued.
 * @param cnt The length of the pointer array \p ptrs.
 * @return The function returns the number of successfully enqueued elements from \p ptrs.
 */
int queue_push_many(struct queue *q, void *ptrs[], size_t cnt);

/** Dequeue up to \p cnt elements from the queue and place them into the array \p ptrs[].
 *
 * @param q A pointer to the queue datastructure.
 * @param[out] ptrs An array with space at least \cnt elements which will receive pointers to the released elements.
 * @param cnt The maximum number of elements which should be dequeued. It defines the size of \p ptrs.
 * @param[in,out] head A pointer to a queue head. The value will be updated to reflect the new head.
 * @return The number of elements which have been dequeued.
 */
int queue_pull_many(struct queue *q, void *ptrs[], size_t cnt);

/** Fill \p ptrs with \p cnt elements of the queue starting at entry \p pos. */
int queue_get_many(struct queue *q, void *ptrs[], size_t cnt);

/** Get the first element in the queue */
static inline int queue_get(struct queue *q, void **ptr)
{
	return queue_get_many(q, ptr, 1);
}

/** Enqueue a new block at the tail of the queue. */
static inline int queue_push(struct queue *q, void **ptr)
{
	return queue_push_many(q, ptr, 1);
}

/** Dequeue the first block at the head of the queue. */
static inline int queue_pull(struct queue *q, void **ptr)
{
	return queue_pull_many(q, ptr, 1);
}

/** Return the number of free slots in a queue
 *
 * Note: This is only an estimate!
 */
int queue_free_slots(struct queue *q);

#endif /* _QUEUE_H_ */