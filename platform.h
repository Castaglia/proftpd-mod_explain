/*
 * ProFTPD - mod_explain: platform limits
 * Copyright (c) 2016 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#ifndef MOD_EXPLAIN_PLATFORM_H
#define MOD_EXPLAIN_PLATFORM_H

#include "mod_explain.h"

long explain_platform_child_max(pool *p);
long explain_platform_iov_max(pool *p);
long explain_platform_name_max(pool *p, const char *path);
long explain_platform_no_trunc(pool *p, const char *path);
long explain_platform_path_max(pool *p, const char *path);
long explain_platform_open_max(pool *p);

void explain_platform_init(pool *p);

#endif /* MOD_EXPLAIN_PLATFORM_H */
