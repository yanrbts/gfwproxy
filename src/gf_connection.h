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
#ifndef __GF_CONNECTION_H__
#define __GF_CONNECTION_H__

#include <gf_core.h>

typedef rstatus_t (*conn_recv_t)(struct context *, struct conn*);
typedef struct msg* (*conn_recv_next_t)(struct context *, struct conn *, bool);
typedef void (*conn_recv_done_t)(struct context *, struct conn *, struct msg *, struct msg *);

typedef rstatus_t (*conn_send_t)(struct context *, struct conn*);
typedef struct msg* (*conn_send_next_t)(struct context *, struct conn *);
typedef void (*conn_send_done_t)(struct context *, struct conn *, struct msg *);

typedef void (*conn_close_t)(struct context *, struct conn *);
typedef bool (*conn_active_t)(const struct conn *);

typedef void (*conn_ref_t)(struct conn *, void *);
typedef void (*conn_unref_t)(struct conn *);

typedef void (*conn_msgq_t)(struct context *, struct conn *, struct msg *);
typedef void (*conn_post_connect_t)(struct context *ctx, struct conn *, struct server *server);
typedef void (*conn_swallow_msg_t)(struct conn *, struct msg *, struct msg *);

struct conn {
    TAILQ_ENTRY(conn)   conn_tqe;        /* link in server_pool / server / free q */
void                *owner;          /* connection owner - server_pool / server */

    int                 sd;              /* socket descriptor */
    int                 family;          /* socket address family */
    socklen_t           addrlen;         /* socket length */
    struct sockaddr     *addr;           /* socket address (ref in server or server_pool) */

    struct msg_tqh      imsg_q;          /* incoming request Q */
    struct msg_tqh      omsg_q;          /* outstanding request Q */
    struct msg          *rmsg;           /* current message being rcvd */
    struct msg          *smsg;           /* current message being sent */

    conn_recv_t         recv;            /* recv (read) handler */
    conn_recv_next_t    recv_next;       /* recv next message handler */
    conn_recv_done_t    recv_done;       /* read done handler */
    conn_send_t         send;            /* send (write) handler */
    conn_send_next_t    send_next;       /* write next message handler */
    conn_send_done_t    send_done;       /* write done handler */
    conn_close_t        close;           /* close handler */
    conn_active_t       active;          /* active? handler */
    conn_post_connect_t post_connect;    /* post connect handler */
    conn_swallow_msg_t  swallow_msg;     /* react on messages to be swallowed */

    conn_ref_t          ref;             /* connection reference handler */
    conn_unref_t        unref;           /* connection unreference handler */

    conn_msgq_t         enqueue_inq;     /* connection inq msg enqueue handler */
    conn_msgq_t         dequeue_inq;     /* connection inq msg dequeue handler */
    conn_msgq_t         enqueue_outq;    /* connection outq msg enqueue handler */
    conn_msgq_t         dequeue_outq;    /* connection outq msg dequeue handler */

    size_t              recv_bytes;      /* received (read) bytes */
    size_t              send_bytes;      /* sent (written) bytes */

    uint32_t            events;          /* connection io events */
    err_t               err;             /* connection errno */
    unsigned            recv_active:1;   /* recv active? */
    unsigned            recv_ready:1;    /* recv ready? */
    unsigned            send_active:1;   /* send active? */
    unsigned            send_ready:1;    /* send ready? */

    unsigned            client:1;        /* client? or server? */
    unsigned            proxy:1;         /* proxy? */
    unsigned            connecting:1;    /* connecting? */
    unsigned            connected:1;     /* connected? */
    unsigned            eof:1;           /* eof? aka passive close? */
    unsigned            done:1;          /* done? aka close? */
    unsigned            redis:1;         /* redis? */
    unsigned            authenticated:1; /* authenticated? */
};

TAILQ_HEAD(conn_tqh, conn);

#endif