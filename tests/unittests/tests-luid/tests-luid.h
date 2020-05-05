/*
 * Copyright (C) 2019 ML!PA Consulting GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Unittests for the ``luid`` module
 *
 * @author      Benjamin Valentin <benjamin.valentin@ml-pa.com>
 */
#ifndef TESTS_LUID_H
#define TESTS_LUID_H

#include "embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The entry point of this test suite.
 */
void tests_luid(void);

/**
 * @brief   Generates tests for luid
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_luid_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_LUID_H */
/** @} */
