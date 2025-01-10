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

struct msg *
req_get(struct conn *conn)
{
    struct msg *msg;

    ASSERT(conn->client && !conn->proxy);

    msg = msg_get(conn, true, conn->redis);
    if (msg == NULL) {
        conn->err = errno;
    }

    return msg;
}

static void
req_log(const struct msg *req)
{
    const struct msg *rsp;              /* peer message (response) */
    int64_t req_time;                   /* time cost for this request */
    const char *peer_str;               /* peer client ip:port */
    uint32_t req_len, rsp_len;          /* request and response length */
    const struct keypos *kpos;

    if (log_loggable(LOG_NOTICE) == 0) {
        return;
    }

    /* a fragment? */
    if (req->frag_id != 0 && req->frag_owner != req) {
        return;
    }

    /* conn close normally? */
    if (req->mlen == 0) {
        return;
    }
    /*
     * there is a race scenario where a requests comes in, the log level is not LOG_NOTICE,
     * and before the response arrives you modify the log level to LOG_NOTICE
     * using SIGTTIN OR SIGTTOU, then req_log() wouldn't have msg->start_ts set
     */
    if (req->start_ts == 0) {
        return;
    }

    req_time = gf_usec_now() - req->start_ts;

    rsp = req->peer;
    req_len = req->mlen;
    rsp_len = (rsp != NULL) ? rsp->mlen : 0;

    if (array_n(req->keys) < 1) {
        return;
    }

    kpos = array_get(req->keys, 0);
    /*
     * FIXME: add backend addr here
     * Maybe we can store addrstr just like server_pool in conn struct
     * when connections are resolved
     */
    peer_str = gf_unresolve_peer_desc(req->owner->sd);

    // req_type = msg_type_string(req->type);

    log_debug(LOG_NOTICE, "req %"PRIu64" done on c %d req_time %"PRIi64".%03"PRIi64
              " narg %"PRIu32" req_len %"PRIu32" rsp_len %"PRIu32
              " key0 '%.*s' peer '%s' done %d error %d",
              req->id, req->owner->sd, req_time / 1000, req_time % 1000,
              req->narg, req_len, rsp_len,
              (int)(kpos->end ? kpos->end - kpos->start : 0), kpos->start,
              peer_str, req->done, req->error);
}


void
req_put(struct msg *msg)
{
    struct msg *pmsg;   /* peer message (response) */

    ASSERT(msg->request);

    req_log(msg);

    pmsg = msg->peer;
    if (pmsg != NULL) {
        ASSERT(!pmsg->request && pmsg->peer == msg);
        msg->peer = NULL;
        pmsg->peer = NULL;
        rsp_put(pmsg);
    }

    msg_tmo_delete(msg);

    msg_put(msg);
}