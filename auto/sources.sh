# Copyright (C) Yan Ruibing

CORE_INCS="src \
           contrib/yaml-0.2.5/include \
           src/hashkit"

CORE_DEPS="src/gf_core.h    \
           src/gf_queue.h   \
           src/gf_util.h    \
           src/gf_log.h     \
           src/gf_string.h  \
           src/gf_array.h   \
           src/gf_mbuf.h    \
           src/gf_rbtree.h  \
           src/gf_stats.h   \
           src/gf_connection.h \
           src/gf_server.h  \
           src/gf_message.h \
           src/event/gf_event.h \
           src/hashkit/gf_hashkit.h \
           src/gf_client.h  \
           src/gf_proxy.h   \
           src/gf_conf.h    \
           src/gf_signal.h"

CORE_SRCS="src/gfw.c        \
           src/gf_util.c    \
           src/gf_log.c     \
           src/gf_string.c  \
           src/gf_array.c   \
           src/gf_mbuf.c    \
           src/gf_rbtree.c  \
           src/gf_stats.c   \
           src/gf_connection.c \
           src/gf_server.c   \
           src/gf_message.c \
           src/event/gf_epoll.c \
           src/event/gf_evport.c \
           src/event/gf_kqueue.c \
           src/hashkit/gf_crc16.c \
           src/hashkit/gf_crc32.c \
           src/hashkit/gf_fnv.c \
           src/hashkit/gf_md5.c \
           src/hashkit/gf_hsieh.c \
           src/hashkit/gf_jenkins.c \
           src/hashkit/gf_ketama.c \
           src/hashkit/gf_modula.c \
           src/hashkit/gf_murmur.c \
           src/hashkit/gf_one_at_a_time.c \
           src/hashkit/gf_random.c \
           src/gf_request.c \
           src/gf_response.c \
           src/gf_client.c  \
           src/gf_proxy.c   \
           src/gf_conf.c    \
           src/gf_signal.c  \
           src/gf_core.c"

UNIX_INCS="$CORE_INCS"

UNIX_DEPS="$CORE_DEPS"

UNIX_SRCS="$CORE_SRCS"

LINUX_DEPS=