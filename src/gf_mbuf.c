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
#include <gf_core.h>

static uint32_t nfree_mbufq;   /* # free mbuf */
static struct mhdr free_mbufq; /* free mbuf q */

static size_t mbuf_chunk_size; /* mbuf chunk size - header + data (const) */
static size_t mbuf_offset;     /* mbuf offset in chunk (const) */

static struct mbuf *
_mbuf_get(void)
{
    struct mbuf *mbuf;
    uint8_t *buf;

    if (!STAILQ_EMPTY(&free_mbufq)) {
        ASSERT(nfree_mbufq > 0);

        mbuf = STAILQ_FIRST(&free_mbufq);
        nfree_mbufq--;
        STAILQ_REMOVE_HEAD(&free_mbufq, next);

        ASSERT(mbuf->magic == MBUF_MAGIC);
        goto done;
    }

    buf = gf_alloc(mbuf_chunk_size);
    if (buf == NULL) {
        return NULL;
    }

    /*
     * mbuf header is at the tail end of the mbuf. This enables us to catch
     * buffer overrun early by asserting on the magic value during get or
     * put operations
     *
     *   <------------- mbuf_chunk_size ------------->
     *   +-------------------------------------------+
     *   |       mbuf data          |  mbuf header   |
     *   |     (mbuf_offset)        | (struct mbuf)  |
     *   +-------------------------------------------+
     *   ^           ^        ^     ^^
     *   |           |        |     ||
     *   \           |        |     |\
     *   mbuf->start \        |     | mbuf->end (one byte past valid bound)
     *                mbuf->pos     \
     *                        \      mbuf
     *                        mbuf->last (one byte past valid byte)
     *
     */
    mbuf = (struct mbuf *)(buf + mbuf_offset);
    mbuf->magic = MBUF_MAGIC;

done:
    STAILQ_NEXT(mbuf, next) = NULL;
    return mbuf;
}

struct mbuf *
mbuf_get(void)
{
    struct mbuf *mbuf;
    uint8_t *buf;

    mbuf = _mbuf_get();
    if (mbuf == NULL) {
        return NULL;
    }

    buf = (uint8_t *)mbuf - mbuf_offset;
    mbuf->start = buf;
    mbuf->end = buf + mbuf_offset;

    ASSERT(mbuf->end - mbuf->start == (int)mbuf_offset);
    ASSERT(mbuf->start < mbuf->end);

    mbuf->pos = mbuf->start;
    mbuf->last = mbuf->start;

    log_debug(LOG_VVERB, "get mbuf %p", mbuf);

    return mbuf;
}

static void
mbuf_free(struct mbuf *mbuf)
{
    uint8_t *buf;

    log_debug(LOG_VVERB, "put mbuf %p len %d", mbuf, (int)(mbuf->last - mbuf->pos));

    ASSERT(STAILQ_NEXT(mbuf, next) == NULL);
    ASSERT(mbuf->magic == MBUF_MAGIC);

    buf = (uint8_t *)mbuf - mbuf_offset;
    gf_free(buf);
}

void
mbuf_put(struct mbuf *mbuf)
{
    log_debug(LOG_VVERB, "put mbuf %p len %d", mbuf, (int)(mbuf->last - mbuf->pos));

    ASSERT(STAILQ_NEXT(mbuf, next) == NULL);
    ASSERT(mbuf->magic == MBUF_MAGIC);

    nfree_mbufq++;
    STAILQ_INSERT_HEAD(&free_mbufq, mbuf, next);
}

/*
 * Rewind the mbuf by discarding any of the read or unread data that it
 * might hold.
 */
void
mbuf_rewind(struct mbuf *mbuf)
{
    mbuf->pos = mbuf->start;
    mbuf->last = mbuf->start;
}

/*
 * Return the length of data in mbuf. Mbuf cannot contain more than
 * 2^32 bytes (4G).
 */
uint32_t
mbuf_length(const struct mbuf *mbuf)
{
    ASSERT(mbuf->last >= mbuf->pos);

    return (uint32_t)(mbuf->last - mbuf->pos);
}

/*
 * Return the remaining space size for any new data in mbuf. Mbuf cannot
 * contain more than 2^32 bytes (4G).
 */
uint32_t
mbuf_size(const struct mbuf *mbuf)
{
    ASSERT(mbuf->end >= mbuf->last);

    return (uint32_t)(mbuf->end - mbuf->last);
}

/*
 * Return the maximum available space size for data in any mbuf. Mbuf cannot
 * contain more than 2^32 bytes (4G).
 */
size_t
mbuf_data_size(void)
{
    return mbuf_offset;
}

/*
 * Insert mbuf at the tail of the mhdr Q
 */
void
mbuf_insert(struct mhdr *mhdr, struct mbuf *mbuf)
{
    STAILQ_INSERT_TAIL(mhdr, mbuf, next);
    log_debug(LOG_VVERB, "insert mbuf %p len %d", mbuf,
            (int)(mbuf->last - mbuf->pos));
}

/*
 * Remove mbuf from the mhdr Q
 */
void
mbuf_remove(struct mhdr *mhdr, struct mbuf *mbuf)
{
    log_debug(LOG_VVERB, "remove mbuf %p len %d", mbuf,
            (int)(mbuf->last - mbuf->pos));

    STAILQ_REMOVE(mhdr, mbuf, mbuf, next);
    STAILQ_NEXT(mbuf, next) = NULL;
}

/*
 * Copy n bytes from memory area pos to mbuf.
 *
 * The memory areas should not overlap and the mbuf should have
 * enough space for n bytes.
 */
void
mbuf_copy(struct mbuf *mbuf, const uint8_t *pos, size_t n)
{
    if (n == 0) {
        return;
    }

    /* mbuf has space for n bytes */
    ASSERT(!mbuf_full(mbuf) && n <= mbuf_size(mbuf));

    /* no overlapping copy */
    ASSERT(pos < mbuf->start || pos >= mbuf->end);

    gf_memcpy(mbuf->last, pos, n);
    mbuf->last += n;
}

/*
 * Split mbuf h into h and t by copying data from h to t. Before
 * the copy, we invoke a precopy handler cb that will copy a predefined
 * string to the head of t.
 *
 * Return new mbuf t, if the split was successful.
 */
struct mbuf *
mbuf_split(struct mhdr *h, uint8_t *pos, mbuf_copy_t cb, void *cbarg)
{
    struct mbuf *mbuf, *nbuf;
    size_t size;

    ASSERT(!STAILQ_EMPTY(h));

    mbuf = STAILQ_LAST(h, mbuf, next);
    ASSERT(pos >= mbuf->pos && pos <= mbuf->last);

    nbuf = mbuf_get();
    if (nbuf == NULL) {
        return NULL;
    }

    if (cb != NULL) {
        /* precopy nbuf */
        cb(nbuf, cbarg);
    }

    /* copy data from mbuf to nbuf */
    size = (size_t)(mbuf->last - pos);
    mbuf_copy(nbuf, pos, size);

    /* adjust mbuf */
    mbuf->last = pos;

    log_debug(LOG_VVERB, "split into mbuf %p len %"PRIu32" and nbuf %p len "
              "%"PRIu32" copied %zu bytes", mbuf, mbuf_length(mbuf), nbuf,
              mbuf_length(nbuf), size);

    return nbuf;
}

void
mbuf_init(size_t cksize)
{
    nfree_mbufq = 0;
    STAILQ_INIT(&free_mbufq);

    mbuf_chunk_size = cksize;

    /* Calculating the offset in this way will 
     * put the head at the end of the memory.*/
    mbuf_offset = mbuf_chunk_size - MBUF_HSIZE;

    log_debug(LOG_DEBUG, "mbuf hsize %d chunk size %zu offset %zu length %zu",
              (int)MBUF_HSIZE, mbuf_chunk_size, mbuf_offset, mbuf_offset);
}

void
mbuf_deinit(void)
{
    while (!STAILQ_EMPTY(&free_mbufq)) {
        struct mbuf *mbuf = STAILQ_FIRST(&free_mbufq);
        mbuf_remove(&free_mbufq, mbuf);
        mbuf_free(mbuf);
        nfree_mbufq--;
    }
    ASSERT(nfree_mbufq == 0);
}
