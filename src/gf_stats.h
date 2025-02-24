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
#ifndef __GF_STATS_H__
#define __GF_STATS_H__

#include <gf_core.h>

#define STATS_POOL_CODEC(ACTION)                                                                                    \
    /* client behavior */                                                                                           \
    ACTION( client_eof,             STATS_COUNTER,      "# eof on client connections")                              \
    ACTION( client_err,             STATS_COUNTER,      "# errors on client connections")                           \
    ACTION( client_connections,     STATS_GAUGE,        "# active client connections")                              \
    /* pool behavior */                                                                                             \
    ACTION( server_ejects,          STATS_COUNTER,      "# times backend server was ejected")                       \
    /* forwarder behavior */                                                                                        \
    ACTION( forward_error,          STATS_COUNTER,      "# times we encountered a forwarding error")                \
    ACTION( fragments,              STATS_COUNTER,      "# fragments created from a multi-vector request")          \

#define STATS_SERVER_CODEC(ACTION)                                                                                  \
    /* server behavior */                                                                                           \
    ACTION( server_eof,             STATS_COUNTER,      "# eof on server connections")                              \
    ACTION( server_err,             STATS_COUNTER,      "# errors on server connections")                           \
    ACTION( server_timedout,        STATS_COUNTER,      "# timeouts on server connections")                         \
    ACTION( server_connections,     STATS_GAUGE,        "# active server connections")                              \
    ACTION( server_ejected_at,      STATS_TIMESTAMP,    "timestamp when server was ejected in usec since epoch")    \
    /* data behavior */                                                                                             \
    ACTION( requests,               STATS_COUNTER,      "# requests")                                               \
    ACTION( request_bytes,          STATS_COUNTER,      "total request bytes")                                      \
    ACTION( responses,              STATS_COUNTER,      "# responses")                                              \
    ACTION( response_bytes,         STATS_COUNTER,      "total response bytes")                                     \
    ACTION( in_queue,               STATS_GAUGE,        "# requests in incoming queue")                             \
    ACTION( in_queue_bytes,         STATS_GAUGE,        "current request bytes in incoming queue")                  \
    ACTION( out_queue,              STATS_GAUGE,        "# requests in outgoing queue")                             \
    ACTION( out_queue_bytes,        STATS_GAUGE,        "current request bytes in outgoing queue")

#define STATS_ADDR      "0.0.0.0"
#define STATS_PORT      22222
#define STATS_INTERVAL  (30 * 1000) /* in msec */

typedef enum stats_type {
    STATS_INVALID,
    STATS_COUNTER,    /* monotonic accumulator */
    STATS_GAUGE,      /* non-monotonic accumulator */
    STATS_TIMESTAMP,  /* monotonic timestamp (in nsec) */
    STATS_SENTINEL
} stats_type_t;

struct stats_metric {
    stats_type_t  type;     /* type */
    struct string name;     /* name(ref) */
    union {
        int64_t   counter;      /* accumulating counter */
        int64_t   timestamp;    /* monotonic timestamp */
    } value;
};

struct stats_server {
    struct string name;     /* server name (ref) */
    struct array  metric;   /* stats_metric[] for server codec */
};

struct stats_pool {
    struct string name;     /* pool name (ref) */
    struct array  metric;   /* stats_metric[] for pool codec */
    struct array  server;   /* stats_server[] */
};

struct stats_buffer {
    size_t   len;   /* buffer length */
    uint8_t  *data; /* buffer data */
    size_t   size;  /* buffer alloc size */
};

struct stats {
    uint16_t            port;            /* stats monitoring port */
    int                 interval;        /* stats aggregation interval */
    struct string       addr;            /* stats monitoring address */ 

    int64_t             start_ts;        /* start timestamp of nutcracker */
    struct stats_buffer buf;             /* output buffer */

    struct array        current;         /* stats_pool[] (a) */
    struct array        shadow;          /* stats_pool[] (b) */
    struct array        sum;             /* stats_pool[] (c = a + b) */

    pthread_t           tid;             /* stats aggregator thread */
    int                 sd;              /* stats descriptor */

    struct string       service_str;     /* service string */
    struct string       service;         /* service */
    struct string       source_str;      /* source string */
    struct string       source;          /* source */
    struct string       version_str;     /* version string */
    struct string       version;         /* version */
    struct string       uptime_str;      /* uptime string */
    struct string       timestamp_str;   /* timestamp string */
    struct string       ntotal_conn_str; /* total connections string */
    struct string       ncurr_conn_str;  /* curr connections string */

    volatile int        aggregate;       /* shadow (b) aggregate? */
    volatile int        updated;         /* current (a) updated? */
};

#define DEFINE_ACTION(_name, _type, _desc) STATS_POOL_##_name,
typedef enum stats_pool_field {
    STATS_POOL_CODEC(DEFINE_ACTION)
    STATS_POOL_NFIELD
} stats_pool_field_t;
#undef DEFINE_ACTION

#define DEFINE_ACTION(_name, _type, _desc) STATS_SERVER_##_name,
typedef enum stats_server_field {
    STATS_SERVER_CODEC(DEFINE_ACTION)
    STATS_SERVER_NFIELD
} stats_server_field_t;
#undef DEFINE_ACTION

#if defined GF_STATS && GF_STATS == 1

#define stats_pool_incr(_ctx, _pool, _name) do {                        \
    _stats_pool_incr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_decr(_ctx, _pool, _name) do {                        \
    _stats_pool_decr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_incr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_incr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_decr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_decr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_set_ts(_ctx, _pool, _name, _val) do {                \
    _stats_pool_set_ts(_ctx, _pool, STATS_POOL_##_name, _val);          \
} while (0)

#define stats_server_incr(_ctx, _server, _name) do {                    \
    _stats_server_incr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_decr(_ctx, _server, _name) do {                    \
    _stats_server_decr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_incr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_incr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_decr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_decr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_set_ts(_ctx, _server, _name, _val) do {            \
     _stats_server_set_ts(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#else

#define stats_pool_incr(_ctx, _pool, _name)
#define stats_pool_decr(_ctx, _pool, _name)
#define stats_pool_incr_by(_ctx, _pool, _name, _val)
#define stats_pool_decr_by(_ctx, _pool, _name, _val)
#define stats_server_incr(_ctx, _server, _name)
#define stats_server_decr(_ctx, _server, _name)
#define stats_server_incr_by(_ctx, _server, _name, _val)
#define stats_server_decr_by(_ctx, _server, _name, _val)

#endif

#define stats_enabled   GF_STATS

void stats_describe(void);

void _stats_pool_incr(struct context *ctx, const struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_decr(struct context *ctx, const struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_incr_by(struct context *ctx, const struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_decr_by(struct context *ctx, const struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_set_ts(struct context *ctx, const struct server_pool *pool, stats_pool_field_t fidx, int64_t val);

void _stats_server_incr(struct context *ctx, const struct server *server, stats_server_field_t fidx);
void _stats_server_decr(struct context *ctx, const struct server *server, stats_server_field_t fidx);
void _stats_server_incr_by(struct context *ctx, const struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_decr_by(struct context *ctx, const struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_set_ts(struct context *ctx, const struct server *server, stats_server_field_t fidx, int64_t val);

struct stats *stats_create(uint16_t stats_port, const char *stats_ip, int stats_interval, const char *source, const struct array *server_pool);
void stats_destroy(struct stats *stats);
void stats_swap(struct stats *stats);

#endif