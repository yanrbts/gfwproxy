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
#ifndef __GF_UTIL_H__
#define __GF_UTIL_H__

#include <stdarg.h>

#ifdef __GNUC__
#define GF_GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
#define GF_GCC_VERSION 0
#endif

#if GF_GCC_VERSION >= 2007
#define GF_ATTRIBUTE_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
#define GF_ATTRIBUTE_FORMAT(type, idx, first)
#endif

#define LF                  (uint8_t)10
#define CR                  (uint8_t)13
#define CRLF                "\x0d\x0a"              // \r\n
#define CRLF_LEN            (sizeof("\x0d\x0a") - 1)

#define NELEMS(a)           ((sizeof(a)) / sizeof((a)[0]))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))

#define SQUARE(d)           ((d) * (d))
#define VAR(s, s2, n)       (((n) < 2) ? 0.0 : ((s2) - SQUARE(s)/(n)) / ((n) - 1))
#define STDDEV(s, s2, n)    (((n) < 2) ? 0.0 : sqrt(VAR((s), (s2), (n))))

#define GF_INET4_ADDRSTRLEN (sizeof("255.255.255.255") - 1)
#define GF_INET6_ADDRSTRLEN \
    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") - 1)
#define GF_INET_ADDRSTRLEN  MAX(GF_INET4_ADDRSTRLEN, GF_INET6_ADDRSTRLEN)
#define GF_UNIX_ADDRSTRLEN  \
    (sizeof(struct sockaddr_un) - offsetof(struct sockaddr_un, sun_path))

#define GF_MAXHOSTNAMELEN   256
#define GF_VERSION_STRING   "0.5.0"

/*
 * Length of 1 byte, 2 bytes, 4 bytes, 8 bytes and largest integral
 * type (uintmax_t) in ascii, including the null terminator '\0'
 *
 * From stdint.h, we have:
 * # define UINT8_MAX	(255)
 * # define UINT16_MAX	(65535)
 * # define UINT32_MAX	(4294967295U)
 * # define UINT64_MAX	(__UINT64_C(18446744073709551615))
 */
#define GF_UINT8_MAXLEN     (3 + 1)
#define GF_UINT16_MAXLEN    (5 + 1)
#define GF_UINT32_MAXLEN    (10 + 1)
#define GF_UINT64_MAXLEN    (20 + 1)
#define GF_UINTMAX_MAXLEN   GF_UINT64_MAXLEN

/*
 * Make data 'd' or pointer 'p', n-byte aligned, where n is a power of 2
 * of 2.
 */
#define GF_ALIGNMENT        sizeof(unsigned long) /* platform word */
#define GF_ALIGN(d, n)      (((d) + (n - 1)) & ~(n - 1))
#define GF_ALIGN_PTR(p, n)  \
    (void *) (((uintptr_t) (p) + ((uintptr_t) n - 1)) & ~((uintptr_t) n - 1))


/*
 * Wrappers for defining custom assert based on whether macro
 * NC_ASSERT_PANIC or NC_ASSERT_LOG was defined at the moment
 * ASSERT was called.
 */
#ifdef GF_ASSERT_PANIC

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        gf_assert(#_x, __FILE__, __LINE__, 1);  \
    }                                           \
} while (0)

#define NOT_REACHED() ASSERT(0)

#elif GF_ASSERT_LOG

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        gf_assert(#_x, __FILE__, __LINE__, 0);  \
    }                                           \
} while (0)

#define NOT_REACHED() ASSERT(0)

#else

#define ASSERT(_x)
#define NOT_REACHED()

#endif

/*
 * Wrapper to workaround well known, safe, implicit type conversion when
 * invoking system calls.
 */
#define gf_gethostname(_name, _len) gethostname((char*)_name, (size_t)_len)
#define gf_atoi(_line, _n)          _gf_atoi((uint8_t *)_line, (size_t)_n)


int gf_set_blocking(int sd);
int gf_set_nonblocking(int sd);
int gf_set_reuseaddr(int sd);
int gf_set_reuseport(int sd);
int gf_set_tcpnodelay(int sd);
int gf_set_linger(int sd, int timeout);
int gf_set_sndbuf(int sd, int size);
int gf_set_rcvbuf(int sd, int size);
int gf_set_tcpkeepalive(int sd);
int gf_get_soerror(int sd);
int gf_get_sndbuf(int sd);
int gf_get_rcvbuf(int sd);
bool gf_valid_port(int n);

/*
 * Memory allocation and free wrappers.
 *
 * These wrappers enables us to loosely detect double free, dangling
 * pointer access and zero-byte alloc.
 */
#define gf_alloc(_s)                    \
    _gf_alloc((size_t)(_s), __FILE__, __LINE__)

#define gf_zalloc(_s)                   \
    _gf_zalloc((size_t)(_s), __FILE__, __LINE__)

#define gf_calloc(_n, _s)               \
    _gf_calloc((size_t)(_n), (size_t)(_s), __FILE__, __LINE__)

#define gf_realloc(_p, _s)              \
    _gf_realloc(_p, (size_t)(_s), __FILE__, __LINE__)

#define gf_free(_p) do {                \
    _gf_free(_p, __FILE__, __LINE__);   \
    (_p) = NULL;                        \
} while (0)

int _gf_atoi(const uint8_t *line, size_t n);
void *_gf_alloc(size_t size, const char *name, int line);
void *_gf_zalloc(size_t size, const char *name, int line);
void *_gf_calloc(size_t nmemb, size_t size, const char *name, int line);
void *_gf_realloc(void *ptr, size_t size, const char *name, int line);
void _gf_free(void *ptr, const char *name, int line);

/*
 * Wrappers to send or receive n byte message on a blocking
 * socket descriptor.
 */
#define gf_sendn(_s, _b, _n)    \
    _gf_sendn(_s, _b, (size_t)(_n))

#define gf_recvn(_s, _b, _n)    \
    _gf_recvn(_s, _b, (size_t)(_n))

/*
 * Wrappers to read or write data to/from (multiple) buffers
 * to a file or socket descriptor.
 */
#define gf_read(_d, _b, _n)     \
    read(_d, _b, (size_t)(_n))

#define gf_readv(_d, _b, _n)    \
    readv(_d, _b, (int)(_n))

#define gf_write(_d, _b, _n)    \
    write(_d, _b, (size_t)(_n))

#define gf_writev(_d, _b, _n)   \
    writev(_d, _b, (int)(_n))

ssize_t _gf_sendn(int sd, const void *vptr, size_t n);
ssize_t _gf_recvn(int sd, void *vptr, size_t n);

void gf_assert(const char *cond, const char *file, int line, int panic);
void gf_stacktrace(int skip_count);
void gf_stacktrace_fd(int fd);

int _gf_scnprintf(char *buf, size_t size, const char *fmt, ...) GF_ATTRIBUTE_FORMAT(printf, 3, 4);
int _gf_vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
int64_t gf_usec_now(void);
int64_t gf_msec_now(void);

/*
 * Address resolution for internet (ipv4 and ipv6) and unix domain
 * socket address.
 */

struct sockinfo {
    int       family;              /* socket address family */
    socklen_t addrlen;             /* socket address length */
    union {
        struct sockaddr_in  in;    /* ipv4 socket address */
        struct sockaddr_in6 in6;   /* ipv6 socket address */
        struct sockaddr_un  un;    /* unix domain address */
    } addr;
};

int gf_resolve(const struct string *name, int port, struct sockinfo *si);
const char *gf_unresolve_addr(struct sockaddr *addr, socklen_t addrlen);
const char *gf_unresolve_peer_desc(int sd);
const char *gf_unresolve_desc(int sd);

#endif