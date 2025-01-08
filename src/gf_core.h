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
#ifndef __GF_CORE_H__
#define __GF_CORE_H__

#include <gf_auto_config.h>

#ifdef HAVE_DEBUG_LOG
# define GF_DEBUG_LOG 1
#endif

#ifdef HAVE_ASSERT_PANIC
# define GF_ASSERT_PANIC 1
#endif

#ifdef HAVE_ASSERT_LOG
# define GF_ASSERT_LOG 1
#endif

#ifdef HAVE_STATS
# define GF_STATS 1
#else
# define GF_STATS 0
#endif

#ifdef HAVE_EPOLL
# define GF_HAVE_EPOLL 1
#elif HAVE_KQUEUE
# define GF_HAVE_KQUEUE 1
#elif HAVE_EVENT_PORTS
# define GF_HAVE_EVENT_PORTS 1
#else
# error missing scalable I/O event notification mechanism
#endif

#ifdef HAVE_LITTLE_ENDIAN
# define GF_LITTLE_ENDIAN 1
#endif

#ifdef HAVE_BACKTRACE
# define GF_HAVE_BACKTRACE 1
#endif

#define GF_OK        0
#define GF_ERROR    -1
#define GF_EAGAIN   -2
#define GF_ENOMEM   -3

/* reserved fds for std streams, log, stats fd, epoll etc. */
#define RESERVED_FDS 32

typedef int rstatus_t; /* return type */
typedef int err_t;     /* error type */

struct array;
struct string;
struct conn;
struct conn_tqh;
struct server;
struct server_pool;
struct mbuf;
struct mhdr;
struct stats;
struct instance;
struct event_base;

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>

#include <gf_string.h>
#include <gf_util.h>
#include <gf_log.h>
#include <gf_array.h>
#include <gf_queue.h>
#include <event/gf_event.h>
#include <gf_stats.h>
#include <gf_mbuf.h>
#include <gf_rbtree.h>
#include <gf_connection.h>
#include <gf_server.h>

struct context {
    uint32_t           id;          /* unique context id */
    struct conf        *cf;         /* configuration */
    struct stats       *stats;      /* stats */

    struct array       pool;        /* server_pool[] */
    struct event_base  *evb;        /* event base */
    int                max_timeout; /* max timeout in msec */
    int                timeout;     /* timeout in msec */

    uint32_t           max_nfd;     /* max # files */
    uint32_t           max_ncconn;  /* max # client connections */
    uint32_t           max_nsconn;  /* max # server connections */
};

struct instance {
    struct context  *ctx;                        /* active context */
    int             log_level;                   /* log level */
    const char      *log_filename;               /* log filename */
    const char      *conf_filename;              /* configuration filename */
    uint16_t        stats_port;                  /* stats monitoring port */
    int             stats_interval;              /* stats aggregation interval */
    const char      *stats_addr;                 /* stats monitoring addr */
    char            hostname[GF_MAXHOSTNAMELEN]; /* hostname */
    size_t          mbuf_chunk_size;             /* mbuf chunk size */
    pid_t           pid;                         /* process id */
    const char      *pid_filename;               /* pid filename */
    unsigned        pidfile:1;                   /* pid file created? */
};

#endif