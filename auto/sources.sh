# Copyright (C) Yan Ruibing

CORE_INCS="src"

CORE_DEPS="src/gf_core.h    \
           src/gf_util.h    \
           src/gf_string.h  \
           src/gf_array.h   \
           src/gf_mbuf.h"

CORE_SRCS="src/gfw.c        \
           src/gf_util.c    \
           src/gf_log.c     \
           src/gf_string.c  \
           src/gf_array.c   \
           src/gf_mbuf.c"

UNIX_INCS="$CORE_INCS"

UNIX_DEPS="$CORE_DEPS"

UNIX_SRCS="$CORE_SRCS"

LINUX_DEPS=