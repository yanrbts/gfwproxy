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

#ifdef GF_HAVE_EPOLL

#include <sys/epoll.h>

struct event_base *
event_base_create(int nevent, event_cb_t cb)
{
    struct event_base *evb;
    int status, ep;
    struct epoll_event *event;

    ASSERT(nevent > 0);

    ep = epoll_create(nevent);
    if (ep < 0) {
        log_error("epoll create of size %d failed: %s", nevent, strerror(errno));
        return NULL;
    }

    event = gf_calloc(nevent, sizeof(*event));
    if (event == NULL) {
        status = close(ep);
        if (status < 0) {
            log_error("close ep %d failed, ignored: %s", ep, strerror(errno));
        }
        return NULL;
    }

    evb = gf_alloc(sizeof(*evb));
    if (evb == NULL) {
        gf_free(event);
        status = close(ep);
        if (status < 0) {
            log_error("close e %d failed, ignored: %s", ep, strerror(errno));
        }
        return NULL;
    }

    evb->ep = ep;
    evb->event = event;
    evb->nevent = nevent;
    evb->cb = cb;

    log_debug(LOG_INFO, "ep %d with nevent %d", evb->ep, evb->nevent);

    return evb;
}

void
event_base_destroy(struct event_base *evb)
{
    int status;

    if (evb == NULL) {
        return;
    }

    ASSERT(evb->ep > 0);

    gf_free(evb->event);

    status = close(evb->ep);
    if (status < 0) {
        log_error("close e %d failed, ignored: %s", evb->ep, strerror(errno));
    }
    evb->ep = -1;

    gf_free(evb);
}

int
event_add_in(struct event_base *evb, struct conn *c)
{
    int status;
    struct epoll_event event;
    int ep = evb->ep;

    ASSERT(ep > 0);
    ASSERT(c != NULL);
    ASSERT(c->sd > 0);

    if (c->recv_active) {
        return 0;
    }

    event.events = (uint32_t)(EPOLLIN | EPOLLET);
    event.data.ptr = c;

    status = epoll_ctl(ep, EPOLL_CTL_MOD, c->sd, &event);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, c->sd,
                  strerror(errno));
    } else {
        c->recv_active = 1;
    }

    return status;
}


int
event_del_in(struct event_base *evb, struct conn *c)
{
    return 0;
}

int
event_add_out(struct event_base *evb, struct conn *c)
{
    int status;
    struct epoll_event event;
    int ep = evb->ep;

    ASSERT(ep > 0);
    ASSERT(c != NULL);
    ASSERT(c->sd > 0);
    ASSERT(c->recv_active);

    if (c->send_active) {
        return 0;
    }

    event.events = (uint32_t)(EPOLLIN | EPOLLOUT | EPOLLET);
    event.data.ptr = c;

    status = epoll_ctl(ep, EPOLL_CTL_MOD, c->sd, &event);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, c->sd,
                  strerror(errno));
    } else {
        c->send_active = 1;
    }

    return status;
}

int
event_del_out(struct event_base *evb, struct conn *c)
{
    int status;
    struct epoll_event event;
    int ep = evb->ep;

    ASSERT(ep > 0);
    ASSERT(c != NULL);
    ASSERT(c->sd > 0);
    ASSERT(c->recv_active);

    if (!c->send_active) {
        return 0;
    }

    event.events = (uint32_t)(EPOLLIN | EPOLLET);
    event.data.ptr = c;

    status = epoll_ctl(ep, EPOLL_CTL_MOD, c->sd, &event);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, c->sd,
                  strerror(errno));
    } else {
        c->send_active = 0;
    }

    return status;
}

int
event_add_conn(struct event_base *evb, struct conn *c)
{
    int status;
    struct epoll_event event;
    int ep = evb->ep;

    ASSERT(ep > 0);
    ASSERT(c != NULL);
    ASSERT(c->sd > 0);

    event.events = (uint32_t)(EPOLLIN | EPOLLOUT | EPOLLET);
    event.data.ptr = c;

    status = epoll_ctl(ep, EPOLL_CTL_ADD, c->sd, &event);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, c->sd,
                  strerror(errno));
    } else {
        c->send_active = 1;
        c->recv_active = 1;
    }

    return status;
}

int
event_del_conn(struct event_base *evb, struct conn *c)
{
    int status;
    int ep = evb->ep;

    ASSERT(ep > 0);
    ASSERT(c != NULL);
    ASSERT(c->sd > 0);

    status = epoll_ctl(ep, EPOLL_CTL_DEL, c->sd, NULL);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, c->sd,
                  strerror(errno));
    } else {
        c->recv_active = 0;
        c->send_active = 0;
    }

    return status;
}

int
event_wait(struct event_base *evb, int timeout)
{
    int ep = evb->ep;
    struct epoll_event *event = evb->event;
    int nevent = evb->nevent;

    ASSERT(ep > 0);
    ASSERT(event != NULL);
    ASSERT(nevent > 0);

    for (;;) {
        int i, nsd;

        nsd = epoll_wait(ep, event, nevent, timeout);
        if (nsd > 0) {
            for (i = 0; i < nsd; i++) {
                struct epoll_event *ev = &evb->event[i];
                uint32_t events = 0;

                log_debug(LOG_VVERB, "epoll %04"PRIX32" triggered on conn %p",
                          ev->events, ev->data.ptr);
                
                if (ev->events & EPOLLERR) {
                    events |= EVENT_ERR;
                }

                if (ev->events & (EPOLLIN | EPOLLHUP)) {
                    events |= EVENT_READ;
                }

                if (ev->events & EPOLLOUT) {
                    events |= EVENT_WRITE;
                }

                if (evb->cb != NULL) {
                    evb->cb(ev->data.ptr, events);
                }
            }
            return nsd;
        }

        if (nsd == 0) {
            if (timeout == -1) {
               log_error("epoll wait on e %d with %d events and %d timeout "
                         "returned no events", ep, nevent, timeout);
                return -1;
            }

            return 0;
        }

        if (errno == EINTR) {
            continue;
        }

        log_error("epoll wait on e %d with %d events failed: %s", ep, nevent,
                  strerror(errno));
        return -1;
    }

    NOT_REACHED();
}

void
event_loop_stats(event_stats_cb_t cb, void *arg)
{
    struct stats *st = arg;
    int status, ep;
    struct epoll_event ev;

    ep = epoll_create(1);
    if (ep < 0) {
        log_error("epoll create failed: %s", strerror(errno));
        return;
    }

    ev.data.fd = st->sd;
    ev.events = EPOLLIN;

    status = epoll_ctl(ep, EPOLL_CTL_ADD, st->sd, &ev);
    if (status < 0) {
        log_error("epoll ctl on e %d sd %d failed: %s", ep, st->sd,
                  strerror(errno));
        goto error;
    }

    for (;;) {
        int n;

        n = epoll_wait(ep, &ev, 1, st->interval);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            log_error("epoll wait on e %d with m %d failed: %s", ep,
                      st->sd, strerror(errno));
            break;
        }

        cb(st, &n);
    }

error:
    status = close(ep);
    if (status < 0) {
        log_error("close e %d failed, ignored: %s", ep, strerror(errno));
    }
    ep = -1;
}

#endif /* GF_HAVE_EPOLL */