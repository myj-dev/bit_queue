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

#include <string.h>
#include "bit_queue.h"

#define BIT_Q_LOCK()		__disable_irq()
#define BIT_Q_UNLOCK()		__enable_irq()

void bit_q_init(bit_q_t * bq, uint8_t * buffer, bq_sz_t bits_num)
{
	if (bq) {
		BIT_Q_LOCK();
		bq->start = 0;
		bq->end = 0;
		bq->q_buf = buffer;
		bq->q_sz = bits_num;
		memset(bq->q_buf, 0, (bits_num + 7) >> 3);
		BIT_Q_UNLOCK();
	}
}

// queue bits data to bit q. bits_buf is MSB first
// return false if queue is full, data will not be queued
bool bit_q_queue_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num)
{
	if ((bit_q_num_bits(bq) + bits_num) > bq->q_sz) {
		return false;
	}

	// add bits to end
	BIT_Q_LOCK();
	if (bq->end & 0x07) {
		// end pointer is not byte aligned, copy data bit to bit
		bq_sz_t i;
		uint8_t mask = 0x80;
		uint8_t q_mask = (uint8_t)(1 << (7 - (bq->end & 0x07)));
		for (i = 0; i < bits_num; i ++) {
			if (bits_buf[i >> 3] & mask) {
				bq->q_buf[bq->end >> 3] |= q_mask;
			}
			else {
				bq->q_buf[bq->end >> 3] &= ~q_mask;
			}
			bq->end ++;
			mask >>= 1;
			q_mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
			}
			if (q_mask == 0) {
				q_mask = 0x80;
			}
		}
	}
	else {
		// byte aligned, use memcpy to accelerate
		memcpy(bq->q_buf, bits_buf, (bits_num + 7) >> 3);
		bq->end += bits_num;
	}

	BIT_Q_UNLOCK();

	return true;
}


bq_sz_t bit_q_dequeue_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num)
{
	bq_sz_t num = bit_q_num_bits(bq) < bits_num? bit_q_num_bits(bq):bits_num;

	BIT_Q_LOCK();
	if (bq->start & 7) {
		// start pointer is not byte aligned
		bq_sz_t i;
		uint8_t mask = 0x80;
		uint8_t q_mask = (uint8_t)(1 << (7 - (bq->start & 0x07)));
		for (i = 0; i < num; i ++) {
			if (bq->q_buf[bq->start >> 3] & q_mask) {
				bits_buf[i >> 3] |= mask;
			}
			else {
				bits_buf[i >> 3] &= ~mask;
			}
			bq->start ++;
			mask >>= 1;
			q_mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
			}
			if (q_mask == 0) {
				q_mask = 0x80;
			}
		}
	}
	else {
		memcpy(bits_buf, bq->q_buf, (bits_num + 7) >> 3);
	}

	BIT_Q_UNLOCK();

	return num;
}


bq_sz_t bit_q_peek_bits(bit_q_t * bq, uint8_t * bits_buf, bq_sz_t bits_num, bq_sz_t index)
{
	bq_sz_t start;
	bq_sz_t num;

	BIT_Q_LOCK();
	start = bq->start + index;
	if (start >= bq->end) {
		BIT_Q_UNLOCK();
		return 0;
	}

	num = (bit_q_num_bits(bq) - index) < bits_num? (bit_q_num_bits(bq) - index):bits_num;
	if (start & 7) {
		// start pointer is not byte aligned
		bq_sz_t i;
		uint8_t mask = 0x80;
		uint8_t q_mask = (uint8_t)(1 << (7 - (start & 0x07)));
		for (i = 0; i < num; i ++) {
			if (bq->q_buf[start >> 3] & q_mask) {
				bits_buf[i >> 3] |= mask;
			}
			else {
				bits_buf[i >> 3] &= ~mask;
			}
			start ++;
			mask >>= 1;
			q_mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
			}
			if (q_mask == 0) {
				q_mask = 0x80;
			}
		}
	}
	else {
		memcpy(bits_buf, bq->q_buf, (bits_num + 7) >> 3);
	}

	BIT_Q_UNLOCK();

	return num;
}

// queue 1 bit data to bit q. bits_buf is MSB first
// return false if queue is full, data will not be queued
bool bit_q_queue(bit_q_t * bq, bool bit)
{
	uint8_t q_mask;

	if ((bit_q_num_bits(bq) + 1) > bq->q_sz) {
		return false;
	}

	BIT_Q_LOCK();
	q_mask = (uint8_t)(1 << (7 - (bq->end & 0x07)));
	if (bit) {
		bq->q_buf[bq->end >> 3] |= q_mask;
	}
	else {
		bq->q_buf[bq->end >> 3] &= ~q_mask;
	}
	bq->end ++;
	BIT_Q_UNLOCK();
	
	return true;
}

bool bit_q_peek(bit_q_t * bq, bool * bit, bq_sz_t index)
{
	uint8_t q_mask;

	if (bit_q_num_bits(bq) == 0) {
		return false;
	}

	BIT_Q_LOCK();
	q_mask = (uint8_t)(1 << (7 - (bq->start & 0x07)));
	if (bq->q_buf[bq->start >> 3] & q_mask) {
		*bit = 1;
	}
	else {
		*bit = 0;
	}
	BIT_Q_UNLOCK();

	return true;
}

bool bit_q_dequeue(bit_q_t * bq, bool * bit)
{
	uint8_t q_mask;

	if (bit_q_num_bits(bq) == 0) {
		return false;
	}

	BIT_Q_LOCK();
	q_mask = (uint8_t)(1 << (7 - (bq->start & 0x07)));
	if (bq->q_buf[bq->start >> 3] & q_mask) {
		*bit = 1;
	}
	else {
		*bit = 0;
	}
	bq->start ++;
	
	if (bit_q_num_bits(bq) == 0) {
		bq->start = 0;
		bq->end = 0;
	}
	BIT_Q_UNLOCK();

	return true;
}

// return current bits number in queue
bq_sz_t bit_q_num_bits(bit_q_t * bq)
{
	return bq->end - bq->start;
}

#if 0
#include <stdio.h>
void bit_q_test(void)
{
	uint8_t bq_buf[16];
	bit_q_t bq;
	bool bit;
	uint8_t deq_test[8];
	bq_sz_t size;
	
	bit_q_init(&bq, bq_buf, 16 * 8);
	
	bit_q_queue(&bq, 1);
	bit_q_queue(&bq, 0);
	bit_q_queue(&bq, 1);
	bit_q_queue(&bq, 1);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_queue(&bq, 0);
	bit_q_queue(&bq, 1);
	bit_q_queue(&bq, 1);
	bit_q_queue(&bq, 0);
	bit_q_queue(&bq, 0);
	bit_q_queue(&bq, 1);
	
	size = bit_q_num_bits(&bq);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	bit_q_dequeue(&bq, &bit);
	//bit_q_dequeue(&bq, &bit);
	size = bit_q_num_bits(&bq);
	bit_q_dequeue(&bq, &bit);
	size = bit_q_num_bits(&bq);
	
	deq_test[0] = 0x39;
	deq_test[1] = 0x62;
	deq_test[2] = 0x5e;
	deq_test[3] = 0x99;
	bit_q_queue_bits(&bq, deq_test, 31);

	deq_test[0] = 0;
	deq_test[1] = 0;
	deq_test[2] = 0;
	deq_test[3] = 0;
	bit_q_dequeue_bits(&bq, deq_test, 31);
	printf("%02X%02X%02X%02X\r\n", deq_test[0], deq_test[1], deq_test[2], deq_test[3]);
}
#endif
