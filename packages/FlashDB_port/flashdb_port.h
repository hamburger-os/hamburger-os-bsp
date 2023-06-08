/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     lvhan       the first version
 */
#ifndef PACKAGES_FLASHDB_PORT_FLASHDB_PORT_H_
#define PACKAGES_FLASHDB_PORT_FLASHDB_PORT_H_

size_t kvdb_get_blob(const char *key, fdb_blob_t blob);
fdb_err_t kvdb_set_blob(const char *key, fdb_blob_t blob);

#endif /* PACKAGES_FLASHDB_PORT_FLASHDB_PORT_H_ */
