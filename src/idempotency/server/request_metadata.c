/*
 * Copyright (c) 2020 YuQing <384681@qq.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include <sys/stat.h>
#include "fastcommon/shared_func.h"
#include "fastcommon/logger.h"
#include "fastcommon/fc_atomic.h"
#include "sf/sf_global.h"
#include "request_metadata.h"

static struct {
    IdempotencyRequestMetadataContext *head;
    IdempotencyRequestMetadataContext *tail;
} ctx_list = {NULL, NULL};

static void *thread_run(void *arg)
{
    IdempotencyRequestMetadataContext *ctx;
    int64_t data_version;

    ctx = ctx_list.head;
    while (ctx != NULL) {
        if (ctx->is_master_callback.func(ctx->is_master_callback.
                    arg, &data_version))
        {
        } else {
        }

        ctx = ctx->next;
    }

    return NULL;
}

int idempotency_request_metadata_init(
        IdempotencyRequestMetadataContext *ctx,
        sf_is_master_callback is_master_callback, void *arg)
{
    int result;

    if ((result=fast_mblock_init_ex1(&ctx->allocator, "req-metadata-info",
                    sizeof(IdempotencyRequestMetadata), 8192, 0,
                    NULL, NULL, true)) != 0)
    {
        return result;
    }

    if ((result=init_pthread_lock(&ctx->lock)) != 0) {
        return result;
    }

    ctx->is_master_callback.func = is_master_callback;
    ctx->is_master_callback.arg = arg;
    ctx->list.head = ctx->list.tail = NULL;

    if (ctx_list.head == NULL) {
        ctx_list.head = ctx;
    } else {
        ctx_list.tail->next = ctx;
    }
    ctx_list.tail = ctx;

    return 0;
}

int idempotency_request_metadata_start()
{
    pthread_t tid;

    return fc_create_thread(&tid, thread_run, NULL,
            SF_G_THREAD_STACK_SIZE);
}

IdempotencyRequestMetadata *idempotency_request_metadata_add(
        IdempotencyRequestMetadataContext *ctx,
        SFRequestMetadata *metadata)
{
    IdempotencyRequestMetadata *idemp_meta;

    if ((idemp_meta=fast_mblock_alloc_object(&ctx->allocator)) == NULL) {
        return NULL;
    }

    idemp_meta->req_id = metadata->req_id;
    idemp_meta->data_version = metadata->data_version;
    idemp_meta->result = -1;
    idemp_meta->next = NULL;
    FC_ATOMIC_INC_EX(idemp_meta->reffer_count, 2);

    PTHREAD_MUTEX_LOCK(&ctx->lock);
    if (ctx->list.head == NULL) {
        ctx->list.head = idemp_meta;
    } else {
        ctx->list.tail->next = idemp_meta;
    }
    ctx->list.tail = idemp_meta;
    PTHREAD_MUTEX_UNLOCK(&ctx->lock);

    return idemp_meta;
}

int idempotency_request_metadata_get(
        IdempotencyRequestMetadataContext *ctx,
        const int64_t req_id, int *err_no)
{
    int result;
    IdempotencyRequestMetadata *meta;

    result = ENOENT;
    PTHREAD_MUTEX_LOCK(&ctx->lock);
    meta = ctx->list.head;
    while (meta != NULL) {
        if (req_id == meta->req_id) {
            result = 0;
            *err_no = meta->result;
        }
        meta = meta->next;
    }
    PTHREAD_MUTEX_UNLOCK(&ctx->lock);

    return result;
}
