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
        log_error("malloc(%zu) failed @ %s:%d", size, name, line);
    } else {
        log_debug(LOG_VVERB, "malloc(%zu) at %p @ %s:%d", size, p, name, line);
    }

    return p;
}

void *_gf_zalloc(size_t size, const char *name, int line) {
    void *p;

    p = _gf_alloc(size, name, line);
    if (p != NULL) {
        memset(p, 0, size);
    }

    return p;
}

void *_gf_calloc(size_t nmemb, size_t size, const char *name, int line) {
    return _gf_zalloc(nmemb * size, name, line);
}

void *_gf_realloc(void *ptr, size_t size, const char *name, int line) {
    void *p;

    ASSERT(size != 0);

    p = realloc(ptr, size);
    if (p == NULL) {
        log_error("realloc(%zu) failed @ %s:%d", size, name, line);
    } else {
        log_debug(LOG_VVERB, "realloc(%zu) at %p @ %s:%d", size, p, name, line);
    }

    return p;
}

void _gf_free(void *ptr, const char *name, int line) {
    ASSERT(ptr != NULL);
    log_debug(LOG_VVERB, "free(%p) @ %s:%d", ptr, name, line);
    free(ptr);
}

void gf_stacktrace(int skip_count) {
#ifdef GF_HAVE_BACKTRACE
    void *stack[64];
    char **symbols;
    int size, i, j;

    /* backtrace()  returns  a backtrace for the calling program, 
     * in the array pointed to by buffer.  A backtrace is the
     * series of currently active function calls for the program.  
     * Each item in the array pointed to  by  buffer  is  of
     * type void *, and is the return address from the corresponding stack frame.  
     * The size argument specifies the maxi‐mum number of addresses that can be stored in buffer.  
     * If the backtrace is larger than size, then  the  addresses
     * corresponding  to  the  size most recent function calls are returned; 
     * to obtain the complete backtrace, make sure
     * that buffer and size are large enough.*/
    size = backtrace(stack, 64);

    /* The backtrace_symbols function translates the information obtained from the
     * backtrace function into an array of strings. The argument buffer should be a
     * pointer to an array of addresses obtained via the backtrace function, and size is the
     * number of entries in that array (the return value of backtrace).*/
    symbols = backtrace_symbols(stack, size);
    if (symbols == NULL) return;

    skip_count++;

    for (i = skip_count, j = 0; i < size; i++, j++) {
        loga("[%d] %s", j, symbols[i]);
    }

    free(symbols);
#endif
}

void gf_stacktrace_fd(int fd) {
#ifdef GF_HAVE_BACKTRACE
    void *stack[64];
    int size;

    size = backtrace(stack, 64);

    /* The backtrace_symbols_fd function performs the same translation as the function
     * backtrace_symbols function. Instead of returning the strings to the caller, it writes
     * the strings to the file descriptor fd, one per line. It does not use the malloc function,
     * and can therefore be used in situations where that function might fail.*/
    backtrace_symbols_fd(stack, size, fd);
#endif
}

void gf_assert(const char *cond, const char *file, int line, int panic) {
    log_error("assert '%s' failed @ (%s, %d)", cond, file, line);
    if (panic) {
        gf_stacktrace(1);
        abort();
    }
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

/*
 * Send n bytes on a blocking descriptor
 */
ssize_t _gf_sendn(int sd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t	nsend;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        nsend = send(sd, ptr, nleft, 0);

        if (nsend < 0) {
            if (errno == EINTR) {
                continue;
            }
            return nsend;
        }

        if (nsend == 0)
            return -1;
        
        nleft -= (size_t)nsend;
        ptr += nsend;
    }
    return (ssize_t)n;
}

/*
 * Recv n bytes from a blocking descriptor
 */
ssize_t _gf_recvn(int sd, void *vptr, size_t n) {
    size_t nleft;
	ssize_t	nrecv;
	char *ptr;

	ptr = vptr;
	nleft = n;
    while (nleft > 0) {
        nrecv = recv(sd, ptr, nleft, 0);

        if (nrecv < 0) {
            if (errno == EINTR) {
                continue;
            }
            return nrecv;
        }

        if (nrecv == 0)
            break;
        
        nleft -= (size_t)nrecv;
        ptr += nrecv;
    }
    return (ssize_t)(n - nleft);
}

/*
 * Return the current time in microseconds since Epoch
 */
int64_t gf_usec_now(void) {
    struct timeval now;
    int64_t usec;
    int status;

    status = gettimeofday(&now, NULL);
    if (status < 0) {
        log_error("gettimeofday failed: %s", strerror(errno));
        return -1;
    }

    usec = (int64_t)now.tv_sec * 1000000LL + (int64_t)now.tv_usec;

    return usec;
}

/*
 * Return the current time in milliseconds since Epoch
 */
int64_t gf_msec_now(void) {
    return gf_usec_now() / 1000LL;
}

static int gf_resolve_inet(const struct string *name, int port, struct sockinfo *si) {
    int status;
    struct addrinfo *ai, *cai; /* head and current addrinfo */
    struct addrinfo hints = {0};
    char *node, service[GF_UINTMAX_MAXLEN];
    bool found = false;

    ASSERT(gf_valid_port(port));

    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = AF_UNSPEC;     /* AF_INET or AF_INET6 */
    hints.ai_socktype = SOCK_STREAM;

    node = (name != NULL) ? (char *)name->data : NULL;
    if (node == NULL) {
        /*
         * If AI_PASSIVE flag is specified in hints.ai_flags, and node is
         * NULL, then the returned socket addresses will be suitable for
         * bind(2)ing a socket that will accept(2) connections. The returned
         * socket address will contain the wildcard IP address.
         */
        hints.ai_flags |= AI_PASSIVE;
    }

    gf_snprintf(service, GF_UINTMAX_MAXLEN, "%d", port);

    /*
     * getaddrinfo() returns zero on success or one of the error codes listed
     * in gai_strerror(3) if an error occurs
     */
    status = getaddrinfo(node, service, &hints, &ai);
    if (status != 0) {
        log_error("address resolution of node '%s' service '%s' failed: %s",
                  node, service, gai_strerror(status));
        return -1;
    }

    /*
     * getaddrinfo() can return a linked list of more than one addrinfo,
     * since we requested for both AF_INET and AF_INET6 addresses and the
     * host itself can be multi-homed. Since we don't care whether we are
     * using ipv4 or ipv6, we just use the first address from this collection
     * in the order in which it was returned.
     *
     * The sorting function used within getaddrinfo() is defined in RFC 3484;
     * the order can be tweaked for a particular system by editing
     * /etc/gai.conf
     */
    for (cai = ai; cai != NULL; cai = cai->ai_next) {
        si->family = cai->ai_family;
        si->addrlen = cai->ai_addrlen;
        gf_memcpy(&si->addr, cai->ai_addr, si->addrlen);
        found = true;
        break;
    }

    freeaddrinfo(ai);

    return found ? 0 : -1;
}

static int gf_resolve_unix(const struct string *name, struct sockinfo *si) {
    struct sockaddr_un *un;

    if (name->len >= GF_UNIX_ADDRSTRLEN) {
        return -1;
    }

    un = &si->addr.un;

    un->sun_family = AF_UNIX;
    gf_memcpy(un->sun_path, name->data, name->len);
    un->sun_path[name->len] = '\0';

    si->family = AF_UNIX;
    si->addrlen = sizeof(*un);
    /* si->addr is an alias of un */

    return 0;
}

/*
 * Resolve a hostname and service by translating it to socket address and
 * return it in si
 *
 * This routine is reentrant
 */
int gf_resolve(const struct string *name, int port, struct sockinfo *si) {
    if (name != NULL && name->data[0] == '/') {
        return gf_resolve_unix(name, si);
    }

    return gf_resolve_inet(name, port, si);
}

/*
 * Unresolve the socket address by translating it to a character string
 * describing the host and service
 *
 * This routine is not reentrant
 */
const char *
gf_unresolve_addr(struct sockaddr *addr, socklen_t addrlen) {
    static char unresolve[NI_MAXHOST + NI_MAXSERV];
    static char host[NI_MAXHOST], service[NI_MAXSERV];
    int status;

    /* The  getnameinfo()  function  is  the inverse of getaddrinfo(3): 
     * it converts a socket address to a corresponding host and service, 
     * in a protocol-independent manner.  It combines the functionality  
     * of  gethostbyaddr(3)  and  getservbyport(3),  but unlike those functions, 
     * getnameinfo() is reentrant and allows programs to eliminate IPv4-versus-IPv6 dependencies.
     * 
     * NI_NUMERICHOST : If set, then the numeric form of the hostname is returned
     * NI_NUMERICSERV : If set, then the numeric form of the service is returned */
    status = getnameinfo(addr, addrlen, host, sizeof(host),
                         service, sizeof(service),
                         NI_NUMERICHOST | NI_NUMERICSERV);
    if (status < 0) {
        return "unknown";
    }

    gf_snprintf(unresolve, sizeof(unresolve), "%s:%s", host, service);

    return unresolve;
}

/*
 * Unresolve the socket descriptor peer address by translating it to a
 * character string describing the host and service
 *
 * This routine is not reentrant
 */
const char *
gf_unresolve_peer_desc(int sd) {
    static struct sockinfo si;
    struct sockaddr *addr;
    socklen_t addrlen;
    int status;

    memset(&si, 0, sizeof(si));
    addr = (struct sockaddr *)&si.addr;
    addrlen = sizeof(si.addr);

    /* getpeername() returns the address of the peer connected to the socket sockfd, 
     * in the buffer pointed to by addr.  The addrlen argument should be 
     * initialized to indicate the amount of space pointed to by addr.  On
     * return  it contains the actual size of the name returned (in bytes).  
     * The name is truncated if the buffer provided is too small.*/
    status = getpeername(sd, addr, &addrlen);
    if (status < 0) {
        return "unknown";
    }

    return gf_unresolve_addr(addr, addrlen);
}

/*
 * Unresolve the socket descriptor address by translating it to a
 * character string describing the host and service
 *
 * This routine is not reentrant
 */
const char *
gf_unresolve_desc(int sd) {
    static struct sockinfo si;
    struct sockaddr *addr;
    socklen_t addrlen;
    int status;

    memset(&si, 0, sizeof(si));
    addr = (struct sockaddr *)&si.addr;
    addrlen = sizeof(si.addr);

    status = getsockname(sd, addr, &addrlen);
    if (status < 0) {
        return "unknown";
    }

    return gf_unresolve_addr(addr, addrlen);
}
