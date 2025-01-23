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
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gf_core.h>

static struct logger logger;

int log_init(int level, const char *filename) {
    struct logger *l = &logger;

    l->level = MAX(LOG_EMERG, MIN(level, LOG_PVERB));;
    l->name = filename;

    if (filename == NULL || !strlen(filename)) {
        l->fd = STDERR_FILENO;
    } else {
        l->fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr("opening log file '%s' failed: %s", filename,
                       strerror(errno));
            return -1;
        }
    }

    return 0;
}

void log_deinit(void) {
    struct logger *l = &logger;

    if (l->fd < 0 || l->fd == STDERR_FILENO) {
        return;
    }

    close(l->fd);
}

void log_reopen(void) {
    struct logger *l = &logger;

    if (l->fd != STDERR_FILENO) {
        close(l->fd);
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr_safe("reopening log file '%s' failed, ignored: %s", l->name,
                       strerror(errno));
        }
    }
}

void log_level_up(void) {
    struct logger *l = &logger;

    if (l->level < LOG_PVERB) {
        l->level++;
        log_safe("up log level to %d", l->level);
    }
}

void log_level_down(void) {
    struct logger *l = &logger;

    if (l->level > LOG_EMERG) {
        l->level--;
        log_safe("down log level to %d", l->level);
    }
}

void log_level_set(int level) {
    struct logger *l = &logger;

    l->level = MAX(LOG_EMERG, MIN(level, LOG_PVERB));
    loga("set log level to %d", l->level);
}

void log_stacktrace(void) {
    struct logger *l = &logger;

    if (l->fd < 0) {
        return;
    }
    gf_stacktrace_fd(l->fd);
}

int log_loggable(int level) {
    struct logger *l = &logger;

    if (level > l->level) {
        return 0;
    }

    return 1;
}

void _log(const char *file, int line, int panic, const char *fmt, ...)
{
    struct logger  *l = &logger;
    int             len, size, errno_save, off, n;
    char            buf[LOG_MAX_LEN];
    char            time_buf[64];
    va_list         args;
    struct timeval  tv;
    time_t          now;
    struct tm       now_tm, *lt_ret;
    const char      *c = ".-*#@!IDVWGP";    

    if (l->fd < 0)
        return;

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    /* get current time */
	now = time(NULL);
	lt_ret = localtime_r(&now, &now_tm);
	gettimeofday(&tv,NULL);

    off = gf_strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S.", lt_ret);
    gf_scnprintf(time_buf+off, sizeof(time_buf)-off, "%03ld", (long)tv.tv_usec/1000);
#ifdef GF_DEBUG_LOG
    len += gf_scnprintf(buf+len, size-len, "%s:%d ", file, line);
#endif

    va_start(args, fmt);
    len += gf_vscnprintf(buf+len, size-len, fmt, args);
    va_end(args);
    buf[len++] = '\0';

    n = dprintf(l->fd, "\033[90m[%d]:%s %c \033[0m%s\n",
            (int)getpid(), time_buf, c[l->level], buf);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;

    if (panic) {
        abort();
    }
}

void _log_stderr(const char *fmt, ...)
{
    struct logger  *l = &logger;
    int             len, size, errno_save;
    char            buf[4 * LOG_MAX_LEN];
    va_list         args;
    ssize_t         n;

    errno_save = errno;
    len = 0;                /* length of output buffer */
    size = 4 * LOG_MAX_LEN; /* size of output buffer */

    va_start(args, fmt);
    len += gf_vscnprintf(buf, size, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = gf_write(STDERR_FILENO, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;
}

/*
 * Hexadecimal dump in the canonical hex + ascii display
 * See -C option in man hexdump
 */
void _log_hexdump(const char *file, int line, const char *data, int datalen,
             const char *fmt, ...)
{
    struct logger  *l = &logger;
    char            buf[8*LOG_MAX_LEN];
    int             i, off, len, size, errno_save;
    ssize_t         n;

    if (l->fd < 0)
        return;
    
    errno_save = errno;
    off = 0;                  /* data offset */
    len = 0;                  /* length of output buffer */
    size = 8*LOG_MAX_LEN;     /* size of output buffer */

    while (datalen != 0 && (len < size-1)) {
        const char *save, *str;
        unsigned char c;
        int savelen;

        len += gf_scnprintf(buf+len, size-len, "%08x  ", off);

        save = data;
        savelen = datalen;

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = (unsigned char)(*data);
            str = (i == 7) ? "  " : " ";
            len += gf_scnprintf(buf+len, size-len, "%02x%s", c, str);
        }

        for (; i < 16; i++) {
            str = (i == 7) ? "  " : " ";
            len += gf_scnprintf(buf+len, size-len, "  %s", str);
        }

        data = save;
        datalen = savelen;

        len += gf_scnprintf(buf + len, size - len, "  |");

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = (unsigned char)(isprint(*data) ? *data : '.');
            len += gf_scnprintf(buf + len, size - len, "%c", c);
        }
        len += gf_scnprintf(buf + len, size - len, "|\n");

        off += 16;
    }

    n = gf_write(l->fd, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    if (len >= size - 1) {
        n = gf_write(l->fd, "\n", 1);
        if (n < 0) {
            l->nerror++;
        }
    }

    errno = errno_save;
}

void _log_safe(const char *fmt, ...) {
    struct logger  *l = &logger;
    int             len, size, errno_save;
    char            buf[LOG_MAX_LEN];
    va_list         args;
    ssize_t         n;

    if (l->fd < 0) {
        return;
    }

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    len += gf_safe_snprintf(buf + len, size - len, "[.......................] ");

    va_start(args, fmt);
    len += gf_safe_vsnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = gf_write(l->fd, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;
}

void
_log_stderr_safe(const char *fmt, ...)
{
    struct logger  *l = &logger;
    int             len = 0, size = LOG_MAX_LEN, errno_save = errno;
    char            buf[LOG_MAX_LEN];
    va_list         args;
    ssize_t         n;

    len += gf_safe_snprintf(buf + len, size - len, "[.......................] ");

    va_start(args, fmt);
    len += gf_safe_vsnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = gf_write(STDERR_FILENO, buf, len);
    if (n < 0) {
        l->nerror++;
    }

    errno = errno_save;
}