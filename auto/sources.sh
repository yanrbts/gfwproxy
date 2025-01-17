# Copyright (C) Yan Ruibing

CORE_INCS="src \
           contrib/yaml-0.2.5/include"

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
           src/gf_client.h"

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
           src/gf_request.c \
           src/gf_client.c"

UNIX_INCS="$CORE_INCS"

UNIX_DEPS="$CORE_DEPS"

UNIX_SRCS="$CORE_SRCS"

LINUX_DEPS=