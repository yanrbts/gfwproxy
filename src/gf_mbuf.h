/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _GF_MBUF_H_
#define _GF_MBUF_H_

#include <gf_core.h>

/* 
 * start                                                      end
 *   |                                                         |
 *   v                                                         v
 * +---+------------------------+---------------------------+---+
 * |   |                        |                           |   |
 * |   |<- pos (read marker)    |<- last (write marker)     |   |
 * |   |                        |                           |   |
 * +---+------------------------+---------------------------+---+
 *   ^                                                         ^
 *   |                                                         |
 * start                                                      end
 */

typedef void (*mbuf_copy_t)(struct mbuf *, void *);

struct mbuf {
    uint32_t           magic;   /* mbuf magic (const) */
    STAILQ_ENTRY(mbuf) next;    /* next mbuf */
    uint8_t            *pos;    /* read marker */
    uint8_t            *last;   /* write marker */
    uint8_t            *start;  /* start of buffer (const) */
    uint8_t            *end;    /* end of buffer (const) */
};

STAILQ_HEAD(mhdr, mbuf);

#define MBUF_MAGIC      0xdeadbeef
#define MBUF_MIN_SIZE   512
#define MBUF_MAX_SIZE   16777216
#define MBUF_SIZE       16384
#define MBUF_HSIZE      sizeof(struct mbuf)

static inline bool
mbuf_empty(const struct mbuf *mbuf)
{
    return mbuf->pos == mbuf->last;
}

static inline bool
mbuf_full(const struct mbuf *mbuf)
{
    return mbuf->last == mbuf->end;
}

void mbuf_init(size_t cksize);
void mbuf_deinit(void);
struct mbuf *mbuf_get(void);
void mbuf_put(struct mbuf *mbuf);
void mbuf_rewind(struct mbuf *mbuf);
uint32_t mbuf_length(const struct mbuf *mbuf);
uint32_t mbuf_size(const struct mbuf *mbuf);
size_t mbuf_data_size(void);
void mbuf_insert(struct mhdr *mhdr, struct mbuf *mbuf);
void mbuf_remove(struct mhdr *mhdr, struct mbuf *mbuf);
void mbuf_copy(struct mbuf *mbuf, const uint8_t *pos, size_t n);
struct mbuf *mbuf_split(struct mhdr *h, uint8_t *pos, mbuf_copy_t cb, void *cbarg);

#endif