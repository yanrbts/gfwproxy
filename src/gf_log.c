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
            log_stderr("opening log file '%s' failed: %s", name,
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
    int             len, size, errno_save;
    char            buf[LOG_MAX_LEN];
    va_list         args;
    ssize_t         n;
    struct timeval  tv;
    time_t          now;
    struct tm       now_tm, *lt_ret;

    if (l->fd < 0)
        return;

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */


    /* get current time */
	now = time(NULL);
	lt_ret = localtime_r(&now, &now_tm);
	gettimeofday(&tv,NULL);

    len += gf_scnprintf(buf+len, size-len, "[%d]", (int)getpid());
    len += gf_strftime(buf+len, size-len, "%Y-%m-%d %H:%M:%S.", lt_ret);
    len += gf_scnprintf(buf+len, size-len, "%03ld", tv.tv_usec/1000);
#ifdef GF_DEBUG_LOG
    len += gf_scnprintf(buf+len, size-len, "%s:%d ", file, line);
#endif

    va_start(args, fmt);
    len += gf_vscnprintf(buf+len, size-len, fmt, args);
    va_end(args);
    buf[len++] = '\n';
}