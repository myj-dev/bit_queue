/*
 * This file is part of the Bit queue library.
 *
 * Copyright (c) 2016-2019, myj-dev, <8823926@sohu.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Bit queue library.
 * Created on: 2020-02-20
 */


#ifndef __BIT_QUEUE_H__
#define __BIT_QUEUE_H__

#include "types.h"

typedef uint32_t			bq_sz_t;

typedef struct _bit_queue_t
{
	// pointer to queue buffer
	uint8_t *q_buf;
	// max number of bits that can be queued
	bq_sz_t q_sz;
	// point to 1st queued element
	bq_sz_t start;
	// point to the empty position for next element
	bq_sz_t end;
} bit_q_t;

// initilize the bit queue
// caller should alloc memory to the parameter 'buffer' pointer
void bit_q_init(bit_q_t * bq, uint8_t * buffer, bq_sz_t bits_num);

// queue bits data to bit q. bits_buf is MSB first
// return false if queue is full, data will not be queued
bool bit_q_queue_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num);

// dequeue bits data from bit q. bits_buf is MSB first
// return number of elements dequeued
bq_sz_t bit_q_dequeue_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num);

// return bits from the queue by specified index without removing them. bits_buf is MSB first
bq_sz_t bit_q_peek_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num, bq_sz_t index);

// queue 1 bit data to bit q.
// return false if queue is full, data will not be queued
bool bit_q_queue(bit_q_t * bq, bool bit);

// dequeue 1 bit data from bit q.
// return false if no element in the queue
bool bit_q_dequeue(bit_q_t * bq, bool * bit);

// return 1 bit data by specified index without removing it.
// return false if no element in the queue
bool bit_q_peek(bit_q_t * bq, bool * bit, bq_sz_t index);

// return current elements number in queue
bq_sz_t bit_q_num_bits(bit_q_t * bq);

#endif
