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
#include <string.h>
#include <gf_core.h>

void string_init(struct string *str) {
    str->len = 0;
    str->data = NULL;
}

void string_deinit(struct string *str) {
    ASSERT((str->len == 0 && str->data == NULL) ||
           (str->len != 0 && str->data != NULL));
    
    if (str->data != NULL) {
        gf_free(str->data);
        string_init(str);
    }
}

bool string_empty(const struct string *str) {
    ASSERT((str->len == 0 && str->data == NULL) ||
           (str->len != 0 && str->data != NULL));
    return str->len == 0;
}

rstatus_t string_duplicate(struct string *dst, const struct string *src) {
    ASSERT(dst->len == 0 && dst->data == NULL);
    ASSERT(src->len != 0 && src->data != NULL);

    dst->data = gf_strndup(src->data, src->len+1);
    if (dst->data == NULL) {
        return GF_ENOMEM;
    }

    dst->len = src->len;
    dst->data[dst->len] = '\0';

    return GF_OK;
}

rstatus_t string_copy(struct string *dst, const uint8_t *src, uint32_t srclen) {
    ASSERT(dst->len == 0 && dst->data == NULL);
    ASSERT(src != NULL && srclen != 0);

    dst->data = gf_strndup(src, srclen+1);
    if (dst->data == NULL) {
        return GF_ENOMEM;
    }

    dst->len = srclen;
    dst->data[dst->len] = '\0';

    return GF_OK;
}

int string_compare(const struct string *s1, const struct string *s2) {
    if (s1->len != s2->len) {
        return s1->len > s2->len ? 1 : -1;
    }

    return gf_strncmp(s1->data, s2->data, s1->len);
}

static const char *const hex = "0123456789abcdef";

static char *
_safe_utoa(int _base, uint64_t val, char *buf)
{
    uint32_t base = (uint32_t)_base;
    *buf-- = 0;
    do {
        *buf-- = hex[val % base];
    } while ((val /= base) != 0);
    return buf + 1;
}

static char *
_safe_itoa(int base, int64_t val, char *buf)
{
    char *orig_buf = buf;
    const int32_t is_neg = (val < 0);
    *buf-- = 0;

    if (is_neg) {
        val = -val;
    }
    if (is_neg && base == 16) {
        int ix;
        val -= 1;
        for (ix = 0; ix < 16; ++ix)
            buf[-ix] = '0';
    }

    do {
        *buf-- = hex[val % base];
    } while ((val /= base) != 0);

    if (is_neg && base == 10) {
        *buf-- = '-';
    }

    if (is_neg && base == 16) {
        int ix;
        buf = orig_buf - 1;
        for (ix = 0; ix < 16; ++ix, --buf) {
            /* *INDENT-OFF* */
            switch (*buf) {
            case '0': *buf = 'f'; break;
            case '1': *buf = 'e'; break;
            case '2': *buf = 'd'; break;
            case '3': *buf = 'c'; break;
            case '4': *buf = 'b'; break;
            case '5': *buf = 'a'; break;
            case '6': *buf = '9'; break;
            case '7': *buf = '8'; break;
            case '8': *buf = '7'; break;
            case '9': *buf = '6'; break;
            case 'a': *buf = '5'; break;
            case 'b': *buf = '4'; break;
            case 'c': *buf = '3'; break;
            case 'd': *buf = '2'; break;
            case 'e': *buf = '1'; break;
            case 'f': *buf = '0'; break;
            }
            /* *INDENT-ON* */
        }
    }
    return buf + 1;
}

static const char *
_safe_check_longlong(const char *fmt, int32_t *have_longlong)
{
    *have_longlong = false;
    if (*fmt == 'l') {
        fmt++;
        if (*fmt != 'l') {
            *have_longlong = (sizeof(long) == sizeof(int64_t));
        } else {
            fmt++;
            *have_longlong = true;
        }
    }
    return fmt;
}

int
_safe_vsnprintf(char *to, size_t size, const char *format, va_list ap)
{
    char *start = to;
    char *end = start + size - 1;

    for ( ; *format; ++format) {
        int32_t have_longlong = false;
        if (*format != '%') {
            if (to == end) { /* end of buffer */
                break;
            }
            *to++ = *format;    /* copy ordinary char */
            continue;
        }
        ++format;               /* skip '%' */

        format = _safe_check_longlong(format, &have_longlong);

        switch (*format) {
        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'p':
            {
                int64_t ival = 0;
                uint64_t uval = 0;
                if (*format == 'p')
                    have_longlong = (sizeof(void *) == sizeof(uint64_t));
                if (have_longlong) {
                    if (*format == 'u') {
                        uval = va_arg(ap, uint64_t);
                    } else {
                        ival = va_arg(ap, int64_t);
                    }
                } else {
                    if (*format == 'u') {
                        uval = va_arg(ap, uint32_t);
                    } else {
                        ival = va_arg(ap, int32_t);
                    }
                }

                {
                    char buff[22];
                    const int base = (*format == 'x' || *format == 'p') ? 16 : 10;

                    /* *INDENT-OFF* */
                    char *val_as_str = (*format == 'u') ?
                        _safe_utoa(base, uval, &buff[sizeof(buff) - 1]) :
                        _safe_itoa(base, ival, &buff[sizeof(buff) - 1]);
                    
                    /* Strip off "ffffffff" if we have 'x' format without 'll' */
                    if (*format == 'x' && !have_longlong && ival < 0) {
                        val_as_str += 8;
                    }

                    while (*val_as_str && to < end) {
                        *to++ = *val_as_str++;
                    }
                    continue;
                }
            }
        case 's':
            {
                const char *val = va_arg(ap, char *);
                if (!val) {
                    val = "(null)";
                }
                while (*val && to < end) {
                    *to++ = *val++;
                }
                continue;
            }
        }
    }
    *to = 0;
    return (int)(to - start);
}

int _safe_snprintf(char *to, size_t n, const char *fmt, ...) {
    int result;
    va_list args;
    va_start(args, fmt);
    result = _safe_vsnprintf(to, n, fmt, args);
    va_end(args);
    return result;
}