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


#ifndef _SF_CLUSTER_CFG_H
#define _SF_CLUSTER_CFG_H

#include "sf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int sf_load_cluster_config_ex(SFClusterConfig *cluster,
        IniFullContext *ini_ctx, const int default_port,
        char *full_server_filename, const int size);

static inline int sf_load_cluster_config(SFClusterConfig *cluster,
        IniFullContext *ini_ctx, const int default_port)
{
    char full_server_filename[PATH_MAX];
    return sf_load_cluster_config_ex(cluster, ini_ctx, default_port,
            full_server_filename, sizeof(full_server_filename));
}

#ifdef __cplusplus
}
#endif

#endif
