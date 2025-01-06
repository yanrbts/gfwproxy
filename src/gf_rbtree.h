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
#ifndef _GF_RBTREE_H_
#define _GF_RBTREE_H_

#define rbtree_red(_node)           ((_node)->color = 1)
#define rbtree_black(_node)         ((_node)->color = 0)
#define rbtree_is_red(_node)        ((_node)->color)
#define rbtree_is_black(_node)      (!rbtree_is_red(_node))
#define rbtree_copy_color(_n1, _n2) ((_n1)->color = (_n2)->color)

struct rbnode {
    struct rbnode *left;     /* left link */
    struct rbnode *right;    /* right link */
    struct rbnode *parent;   /* parent link */
    int64_t       key;       /* key for ordering */
    void          *data;     /* opaque data */
    uint8_t       color;     /* red | black */
};

struct rbtree {
    struct rbnode *root;     /* root node */
    struct rbnode *sentinel; /* nil node */
};

void rbtree_node_init(struct rbnode *node);
void rbtree_init(struct rbtree *tree, struct rbnode *node);
struct rbnode *rbtree_min(const struct rbtree *tree);
void rbtree_insert(struct rbtree *tree, struct rbnode *node);
void rbtree_delete(struct rbtree *tree, struct rbnode *node);

#endif
