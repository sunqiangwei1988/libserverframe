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

#ifndef _SF_IOV_H
#define _SF_IOV_H

#include "fastcommon/shared_func.h"
#include "sf_types.h"

#define SF_IOV_FIXED_SIZE    256

typedef struct sf_dynamic_iov_array {
    struct iovec holder[SF_IOV_FIXED_SIZE];
    struct iovec *ptr;

    struct {
        struct iovec *iov;
        int cnt;
    } input;

    struct iovec *iov;
    int cnt;
} SFDynamicIOVArray;

#define sf_iova_init(iova, _iov, _cnt)    \
    (iova).iov = (iova).input.iov = _iov; \
    (iova).cnt = (iova).input.cnt = _cnt

#define sf_iova_destroy(iova) \
    if ((iova).iov != (iova).input.iov && \
            (iova).ptr != (iova).holder)  \
            free((iova).ptr)

#ifdef __cplusplus
extern "C" {
#endif

static inline int sf_iova_check_alloc(SFDynamicIOVArray *iova)
{
    if (iova->iov == iova->input.iov) {
        if (iova->input.cnt <= SF_IOV_FIXED_SIZE) {
            iova->ptr = iova->holder;
        } else {
            iova->ptr = fc_malloc(iova->input.cnt *
                    sizeof(struct iovec));
            if (iova->ptr == NULL) {
                return ENOMEM;
            }
        }

        memcpy(iova->ptr, iova->input.iov, iova->input.cnt *
                sizeof(struct iovec));
        iova->iov = iova->ptr;
    }

    return 0;
}

int sf_iova_consume(SFDynamicIOVArray *iova, const int consume_len);

int sf_iova_first_slice(SFDynamicIOVArray *iova, const int slice_len);

int sf_iova_next_slice(SFDynamicIOVArray *iova,
        const int consume_len, const int slice_len);

#ifdef __cplusplus
}
#endif

#endif
