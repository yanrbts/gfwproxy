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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <gf_core.h>

#ifdef GF_HAVE_BACKTRACE
# include <execinfo.h>
#endif

int gf_set_blocking(int sd) {
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }
    return fcntl(sd, F_SETFL, flags & ~O_NONBLOCK);
}

int gf_set_nonblocking(int sd) {
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}

int gf_set_reuseaddr(int sd) {
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

/* Permits multiple AF_INET or AF_INET6 sockets to be bound to an identical socket address.  This op‐
 * tion must be set on each socket (including the first socket)  prior  to  calling  bind(2)  on  the
 * socket.  To prevent port hijacking, all of the processes binding to the same address must have the
 * same effective UID.  This option can be employed with both TCP and UDP sockets.

 * For TCP sockets, this option allows accept(2) load distribution in a multi-threaded server  to  be
 * improved by using a distinct listener socket for each thread.  This provides improved load distri‐
 * bution as compared to traditional techniques such using a single accept(2)ing thread that distrib‐
 * utes connections, or having multiple threads that compete to accept(2) from the same socket.*/
int gf_set_reuseport(int sd) {
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);
#ifdef SO_REUSEPORT
    return setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &reuse, len);
#endif
    return -1;
}

/*
 * Disable Nagle algorithm on TCP socket.
 *
 * This option helps to minimize transmit latency by disabling coalescing
 * of data to fill up a TCP segment inside the kernel. Sockets with this
 * option must use readv() or writev() to do data transfer in bulk and
 * hence avoid the overhead of small packets.
 */
int gf_set_tcpnodelay(int sd) {
    int nodelay;
    socklen_t len;

    nodelay = 1;
    len = sizeof(nodelay);

    return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &nodelay, len);
}

/* When enabled, a close(2) or shutdown(2) will not return until all queued messages for  the  socket
 * have  been  successfully sent or the linger timeout has been reached.  Otherwise, the call returns
 * immediately and the closing is done in the background.  When the  socket  is  closed  as  part  of
 * exit(2), it always lingers in the background.*/
int gf_set_linger(int sd, int timeout) {
    struct linger linger;
    socklen_t len;

    linger.l_onoff = 1;
    linger.l_linger = timeout;

    len = sizeof(linger);

    return setsockopt(sd, SOL_SOCKET, SO_LINGER, &linger, len);
}

int gf_set_tcpkeepalive(int sd) {
    int val = 1;
    return setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
}

int gf_set_sndbuf(int sd, int size) {
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, len);
}

int gf_set_rcvbuf(int sd, int size) {
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, len);
}

int gf_get_soerror(int sd) {
    int status, err;
    socklen_t len;

    err = 0;
    len = sizeof(err);

    status = getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status == 0) {
        errno = err;
    }

    return status;
}

int gf_get_sndbuf(int sd) {
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int gf_get_rcvbuf(int sd) {
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int _gf_atoi(const uint8_t *line, size_t n) {
    int value;

    if (n == 0) {
        return -1;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return -1;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return -1;
    }

    return value;
}

bool gf_valid_port(int n) {
    if (n < 1 || n > UINT16_MAX) {
        return false;
    }
    return true;
}

void *_gf_alloc(size_t size, const char *name, int line) {
    void *p;

    ASSERT(size != 0);

    p = malloc(size);
    if (p == NULL) {

    } else {

    }

    return p;
}

int _gf_vscnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    int n;

    n = vsnprintf(buf, size, fmt, args);

    /*
     * The return value is the number of characters which would be written
     * into buf not including the trailing '\0'. If size is == 0 the
     * function returns 0.
     *
     * On error, the function also returns 0. This is to allow idiom such
     * as len += _vscnprintf(...)
     *
     * See: http://lwn.net/Articles/69419/
     */
    if (n <= 0) {
        return 0;
    }

    if (n < (int) size) {
        return n;
    }

    return (int)(size - 1);
}

int _gf_scnprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    int n;

    va_start(args, fmt);
    n = _gf_vscnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}