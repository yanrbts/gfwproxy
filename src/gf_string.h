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
#ifndef _GF_STRING_H_
#define _GF_STRING_H_

#include <string.h>
#include <sys/types.h>
#include <stdarg.h>

#include <gf_core.h>

struct string {
    uint32_t len;   /* string length */
    uint8_t  *data; /* string data */
};

/*
 * snprintf(s, n, ...) will write at most n - 1 of the characters printed into
 * the output string; the nth character then gets the terminating `\0'; if
 * the return value is greater than or equal to the n argument, the string
 * was too short and some of the printed characters were discarded; the output
 * is always null-terminated.
 *
 * Note that, the return value of snprintf() is always the number of characters
 * that would be printed into the output string, assuming n were limited not
 * including the trailing `\0' used to end output.
 *
 * scnprintf(s, n, ...) is same as snprintf() except, it returns the number
 * of characters printed into the output string not including the trailing '\0'
 */
#define gf_snprintf(_s, _n, ...)        \
    snprintf((char *)(_s), (size_t)(_n), __VA_ARGS__)

#define gf_scnprintf(_s, _n, ...)       \
    _gf_scnprintf((char *)(_s), (size_t)(_n), __VA_ARGS__)

#define gf_vsnprintf(_s, _n, _f, _a)    \
    vsnprintf((char *)(_s), (size_t)(_n), _f, _a)

#define gf_vscnprintf(_s, _n, _f, _a)   \
    _gf_vscnprintf((char *)(_s), (size_t)(_n), _f, _a)

#define gf_strftime(_s, _n, fmt, tm)        \
    (int)strftime((char *)(_s), (size_t)(_n), fmt, tm)

#endif