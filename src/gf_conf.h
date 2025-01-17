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
#ifndef __GF_CONF_H__
#define __GF_CONF_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <yaml.h>

#include <gf_core.h>
#include <hashkit/gf_hashkit.h>

#define CONF_OK                             (void *) NULL
#define CONF_ERROR                          (void *) "has an invalid value"

#define CONF_ROOT_DEPTH                     1
#define CONF_MAX_DEPTH                      CONF_ROOT_DEPTH + 1

#define CONF_DEFAULT_ARGS                   3
#define CONF_DEFAULT_POOL                   8
#define CONF_DEFAULT_SERVERS                8

#define CONF_UNSET_NUM                      -1
#define CONF_UNSET_PTR                      NULL
#define CONF_UNSET_HASH                     (hash_type_t)-1
#define CONF_UNSET_DIST                     (dist_type_t)-1

#define CONF_DEFAULT_HASH                    HASH_FNV1A_64
#define CONF_DEFAULT_DIST                    DIST_KETAMA
#define CONF_DEFAULT_TIMEOUT                 -1
#define CONF_DEFAULT_LISTEN_BACKLOG          512
#define CONF_DEFAULT_CLIENT_CONNECTIONS      0
#define CONF_DEFAULT_REDIS                   false
#define CONF_DEFAULT_REDIS_DB                0
#define CONF_DEFAULT_PRECONNECT              false
#define CONF_DEFAULT_AUTO_EJECT_HOSTS        false
#define CONF_DEFAULT_SERVER_RETRY_TIMEOUT    30 * 1000      /* in msec */
#define CONF_DEFAULT_SERVER_FAILURE_LIMIT    2
#define CONF_DEFAULT_SERVER_CONNECTIONS      1
#define CONF_DEFAULT_KETAMA_PORT             11211
#define CONF_DEFAULT_TCPKEEPALIVE            false
#define CONF_DEFAULT_REUSEPORT		         false

struct conf_listen {
    struct string   pname;   /* listen: as "hostname:port" */
    struct string   name;    /* hostname:port */
    int             port;    /* port */
    mode_t          perm;    /* socket permissions */
    struct sockinfo info;    /* listen socket info */
    unsigned        valid:1; /* valid? */
};

struct conf_server {
    struct string   pname;      /* server: as "hostname:port:weight" */
    struct string   name;       /* hostname:port or [name] */
    struct string   addrstr;    /* hostname */
    int             port;       /* port */
    int             weight;     /* weight */
    struct sockinfo info;       /* connect socket info */
    unsigned        valid:1;    /* valid? */
};

struct conf_pool {
    struct string      name;                  /* pool name (root node) */
    struct conf_listen listen;                /* listen: */
    hash_type_t        hash;                  /* hash: */
    struct string      hash_tag;              /* hash_tag: */
    dist_type_t        distribution;          /* distribution: */
    int                timeout;               /* timeout: */
    int                backlog;               /* backlog: */
    int                client_connections;    /* client_connections: */
    int                tcpkeepalive;          /* tcpkeepalive: */
    int                redis;                 /* redis: */
    struct string      redis_auth;            /* redis_auth: redis auth password (matches requirepass on redis) */
    int                redis_db;              /* redis_db: redis db */
    int                preconnect;            /* preconnect: */
    int                auto_eject_hosts;      /* auto_eject_hosts: */
    int                server_connections;    /* server_connections: */
    int                server_retry_timeout;  /* server_retry_timeout: in msec */
    int                server_failure_limit;  /* server_failure_limit: */
    struct array       server;                /* servers: conf_server[] */
    unsigned           valid:1;               /* valid? */
    int                reuseport;             /* set SO_REUSEPORT to socket */
};

struct conf {
    const char    *fname;           /* file name (ref in argv[]) */
    FILE          *fh;              /* file handle */
    struct array  arg;              /* string[] (parsed {key, value} pairs) */
    struct array  pool;             /* conf_pool[] (parsed pools) */
    uint32_t      depth;            /* parsed tree depth */
    yaml_parser_t parser;           /* yaml parser */
    yaml_event_t  event;            /* yaml event */
    yaml_token_t  token;            /* yaml token */
    unsigned      seq:1;            /* sequence? */
    unsigned      valid_parser:1;   /* valid parser? */
    unsigned      valid_event:1;    /* valid event? */
    unsigned      valid_token:1;    /* valid token? */
    unsigned      sound:1;          /* sound? */
    unsigned      parsed:1;         /* parsed? */
    unsigned      valid:1;          /* valid? */
};

struct command {
    struct string name;
    const char    *(*set)(struct conf *cf, const struct command *cmd, void *data);
    int           offset;
};

#define null_command { null_string, NULL, 0 }

const char *conf_set_string(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_listen(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_add_server(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_num(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_bool(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_hash(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_distribution(struct conf *cf, const struct command *cmd, void *conf);
const char *conf_set_hashtag(struct conf *cf, const struct command *cmd, void *conf);

rstatus_t conf_server_each_transform(void *elem, void *data);
rstatus_t conf_pool_each_transform(void *elem, void *data);

struct conf *conf_create(const char *filename);
void conf_destroy(struct conf *cf);

#endif