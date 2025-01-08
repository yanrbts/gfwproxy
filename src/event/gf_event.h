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

#ifndef __GF_EVENT_H__
#define __GF_EVENT_H__

#include <gf_core.h>

#define EVENT_SIZE  1024

#define EVENT_READ  0x0000ff
#define EVENT_WRITE 0x00ff00
#define EVENT_ERR   0xff0000

typedef int (*event_cb_t)(void *, uint32_t);
typedef void (*event_stats_cb_t)(void *, void *);

#ifdef GF_HAVE_KQUEUE

struct event_base {
    int           kq;          /* kernel event queue descriptor */

    struct kevent *change;     /* change[] - events we want to monitor */
    int           nchange;     /* # change */

    struct kevent *event;      /* event[] - events that were triggered */
    int           nevent;      /* # event */
    int           nreturned;   /* # event placed in event[] */
    int           nprocessed;  /* # event processed from event[] */

    event_cb_t    cb;          /* event callback */
};

#elif GF_HAVE_EPOLL

struct event_base {
    int                ep;      /* epoll descriptor */

    struct epoll_event *event;  /* event[] - events that were triggered */
    int                nevent;  /* # event */

    event_cb_t         cb;      /* event callback */
};

#elif GF_HAVE_EVENT_PORTS

#include <port.h>

struct event_base {
    int          evp;     /* event port descriptor */

    port_event_t *event;  /* event[] - events that were triggered */
    int          nevent;  /* # event */

    event_cb_t   cb;      /* event callback */
};

#else
# error missing scalable I/O event notification mechanism
#endif

struct event_base *event_base_create(int size, event_cb_t cb);
void event_base_destroy(struct event_base *evb);

int event_add_in(struct event_base *evb, struct conn *c);
int event_del_in(struct event_base *evb, struct conn *c);
int event_add_out(struct event_base *evb, struct conn *c);
int event_del_out(struct event_base *evb, struct conn *c);
int event_add_conn(struct event_base *evb, struct conn *c);
int event_del_conn(struct event_base *evb, struct conn *c);
int event_wait(struct event_base *evb, int timeout);
void event_loop_stats(event_stats_cb_t cb, void *arg);

#endif /* _NC_EVENT_H */
