/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-21     MurphyZhao   the first version
 */

#include <rtthread.h>
#include "utest.h"

#include <stdio.h>
#include <string.h>

#define TEST_VALUE (0xAA) /* None ascii */
#define INSERT_POSITION (5)

static char cmp_buf[INSERT_POSITION];
static char test_buf[32];
static int test_size = 0;

#define MIN(a,b) (((a)<(b))?(a):(b))

#define TEST_CHECK(EXP, SIZE) \
    uassert_true(ret == (sizeof(EXP)-1)); \
    uassert_true(memcmp(test_buf, cmp_buf, INSERT_POSITION) == 0); \
    if (SIZE >= 0) { \
        uassert_true(memcmp(test_buf + INSERT_POSITION, EXP, SIZE) == 0); \
        uassert_true(test_buf[INSERT_POSITION + SIZE] == '\0'); \
    } \
    uassert_true(memcmp(test_buf + INSERT_POSITION + SIZE + 1, cmp_buf, INSERT_POSITION) == 0);

#define _TEST_SNPRINTF(FLG, EXP, SIZE, ...) \
    rt_memset(cmp_buf, TEST_VALUE, sizeof(cmp_buf)); \
    rt_memset(test_buf, TEST_VALUE, sizeof(test_buf)); \
    ret = 0; \
    test_size=(int)(MIN((SIZE), sizeof(EXP))-1); \
    ret = (FLG) ? rt_snprintf((char*)(test_buf + INSERT_POSITION), (rt_size_t)(SIZE), __VA_ARGS__) : snprintf((char*)(test_buf + INSERT_POSITION), (SIZE), __VA_ARGS__); \
    TEST_CHECK(EXP, test_size)

#define _TEST_SNPRINTF_NULL(FLG, EXP, ...) \
    rt_memset(cmp_buf, TEST_VALUE, sizeof(cmp_buf)); \
    rt_memset(test_buf, TEST_VALUE, sizeof(test_buf)); \
    ret = 0; \
    ret = (FLG) ? rt_snprintf(0, 0, __VA_ARGS__) : snprintf(0, 0, __VA_ARGS__); \
    uassert_true(ret == (sizeof(EXP)-1));

#define TEST_SPRINTF(FLG, EXP, ...) \
    rt_memset(cmp_buf, TEST_VALUE, sizeof(cmp_buf)); \
    rt_memset(test_buf, TEST_VALUE, sizeof(test_buf)); \
    ret = 0; \
    ret = (FLG) ? rt_sprintf((char*)(test_buf + INSERT_POSITION), __VA_ARGS__) : sprintf((char*)(test_buf + INSERT_POSITION), __VA_ARGS__); \
    TEST_CHECK(EXP, ret); \
    _TEST_SNPRINTF(FLG, EXP, sizeof(EXP) + 1, __VA_ARGS__); \
    _TEST_SNPRINTF(FLG, EXP, sizeof(EXP)    , __VA_ARGS__); \
    _TEST_SNPRINTF(FLG, EXP, sizeof(EXP) - 1, __VA_ARGS__); \
    _TEST_SNPRINTF(FLG, EXP, 1,               __VA_ARGS__); \
    if (!(FLG)) { \
        _TEST_SNPRINTF(FLG, EXP, 0,           __VA_ARGS__); \
    } \
    _TEST_SNPRINTF(FLG, EXP, sizeof(EXP)+INSERT_POSITION, __VA_ARGS__); \
    _TEST_SNPRINTF(FLG, EXP, 1,               __VA_ARGS__); \
    _TEST_SNPRINTF_NULL(FLG, EXP, __VA_ARGS__);

static void test_rt_sprintf(void)
{
    int ret = 0;

    TEST_SPRINTF(RT_TRUE, "rt-thread", "rt-thread");

    TEST_SPRINTF(RT_TRUE, "12",     "%d",  12);
    TEST_SPRINTF(RT_TRUE, "    -4", "%6d", -4);

    TEST_SPRINTF(RT_TRUE, "42",     "%i",  42);
    // TEST_SPRINTF(RT_TRUE, "",       "%.0i", 0);

    /* Octal output is not supported */
    // TEST_SPRINTF(RT_TRUE, "52",     "%o",  42);
    // TEST_SPRINTF(RT_TRUE, "",       "%.0o", 0);
    // TEST_SPRINTF(RT_TRUE, "012",    "%03o", 10);

    TEST_SPRINTF(RT_TRUE, "42",     "%u",  42);
    // TEST_SPRINTF(RT_TRUE, "",       "%.0u", 0);

    TEST_SPRINTF(RT_TRUE, "2a",     "%x",  42);
    // TEST_SPRINTF(RT_TRUE, "",       "%.0x", 0);

    TEST_SPRINTF(RT_TRUE, "2A",     "%X",  42);
    // TEST_SPRINTF(RT_TRUE, "",       "%.0X", 0);

    /* Float output is not supported */
    // TEST_SPRINTF(RT_TRUE, "0.000003", "%f",     3e-6);
    // TEST_SPRINTF(RT_TRUE, "0.030000", "%f",     3e-2);
    // TEST_SPRINTF(RT_TRUE, "0.12",     "%.2f",   0.12);

    // TEST_SPRINTF(RT_TRUE, "12.23",    "%5.2f", 12.23);
    // TEST_SPRINTF(RT_TRUE, "12.23",    "%5.4g", 12.23);
    // TEST_SPRINTF(RT_TRUE, " 12.2",    "%5.3g", 12.23);

    // TEST_SPRINTF(RT_TRUE, "01.23",    "%05.2f", 1.23);
    // TEST_SPRINTF(RT_TRUE, "001.2",    "%05.2g", 1.23);


    TEST_SPRINTF(RT_TRUE, "   1",    "%*i", 4, 1);
    TEST_SPRINTF(RT_TRUE, "   1",     "%4i",   1);
    TEST_SPRINTF(RT_TRUE, "1   ",     "%-4i",  1);
    TEST_SPRINTF(RT_TRUE, "  +1",     "%+4i",  1);
    TEST_SPRINTF(RT_TRUE, "+1  ",     "%-+4i", 1);
    TEST_SPRINTF(RT_TRUE, " 1  ",     "%- 4i", 1);
    TEST_SPRINTF(RT_TRUE, "0001",     "%04i",  1);
    TEST_SPRINTF(RT_TRUE, "+001",     "%+04i", 1);

    /* Not support */
    // TEST_SPRINTF(RT_TRUE, "0x1",      "%#x",   1);

    TEST_SPRINTF(RT_TRUE, "abcX",     "%2sX",  "abc");
    TEST_SPRINTF(RT_TRUE, "abcX",     "%-2sX", "abc");

    TEST_SPRINTF(RT_TRUE, "001234",   "%.6u",  1234);
    TEST_SPRINTF(RT_TRUE, "-001234",  "%.6i",  -1234);
    TEST_SPRINTF(RT_TRUE, "  1234",   "%6u",   1234);
    TEST_SPRINTF(RT_TRUE, " -1234",   "%6i",   -1234);
    TEST_SPRINTF(RT_TRUE, "001234",   "%06u",  1234);
    TEST_SPRINTF(RT_TRUE, "-01234",   "%06i",  -1234);
    TEST_SPRINTF(RT_TRUE, "1234  ",   "%-6u",  1234);

    TEST_SPRINTF(RT_TRUE, "-1234 ",   "%-6i",  -1234);
    TEST_SPRINTF(RT_TRUE, "1234",     "%.6s",  "1234");
    TEST_SPRINTF(RT_TRUE, "  1234",   "%6s",   "1234");
    TEST_SPRINTF(RT_TRUE, "1234  ",   "%-6s",  "1234");
    TEST_SPRINTF(RT_TRUE, " 01234",   "%6.5u", 1234);
    TEST_SPRINTF(RT_TRUE, "-01234",   "%6.5i", -1234);
    TEST_SPRINTF(RT_TRUE, "  1234",   "%6.5s", "1234");

    TEST_SPRINTF(RT_TRUE, "rt-thread", "%s", "rt-thread");
}


static void test_sprintf(void)
{
    int ret = 0;

    TEST_SPRINTF(RT_FALSE, "rt-thread", "rt-thread");

    TEST_SPRINTF(RT_FALSE, "12",     "%d",  12);
    TEST_SPRINTF(RT_FALSE, "    -4", "%6d", -4);

    TEST_SPRINTF(RT_FALSE, "42",     "%i",  42);
    TEST_SPRINTF(RT_FALSE, "",       "%.0i", 0);

    TEST_SPRINTF(RT_FALSE, "52",     "%o",  42);
    TEST_SPRINTF(RT_FALSE, "",       "%.0o", 0);
    TEST_SPRINTF(RT_FALSE, "012",    "%03o", 10);

    TEST_SPRINTF(RT_FALSE, "42",     "%u",  42);
    TEST_SPRINTF(RT_FALSE, "",       "%.0u", 0);

    TEST_SPRINTF(RT_FALSE, "2a",     "%x",  42);
    TEST_SPRINTF(RT_FALSE, "",       "%.0x", 0);

    TEST_SPRINTF(RT_FALSE, "2A",     "%X",  42);
    TEST_SPRINTF(RT_FALSE, "",       "%.0X", 0);

    TEST_SPRINTF(RT_FALSE, "0.000003", "%f",     3e-6);
    TEST_SPRINTF(RT_FALSE, "0.030000", "%f",     3e-2);
    TEST_SPRINTF(RT_FALSE, "0.12",     "%.2f",   0.12);

    TEST_SPRINTF(RT_FALSE, "12.23",    "%5.2f", 12.23);
    TEST_SPRINTF(RT_FALSE, "12.23",    "%5.4g", 12.23);
    TEST_SPRINTF(RT_FALSE, " 12.2",    "%5.3g", 12.23);

    TEST_SPRINTF(RT_FALSE, "01.23",    "%05.2f", 1.23);
    TEST_SPRINTF(RT_FALSE, "001.2",    "%05.2g", 1.23);

    TEST_SPRINTF(RT_FALSE, "   1",    "%*i", 4, 1);
    TEST_SPRINTF(RT_FALSE, "   1",     "%4i",   1);
    TEST_SPRINTF(RT_FALSE, "1   ",     "%-4i",  1);
    TEST_SPRINTF(RT_FALSE, "  +1",     "%+4i",  1);
    TEST_SPRINTF(RT_FALSE, "+1  ",     "%-+4i", 1);
    TEST_SPRINTF(RT_FALSE, " 1  ",     "%- 4i", 1);
    TEST_SPRINTF(RT_FALSE, "0001",     "%04i",  1);
    TEST_SPRINTF(RT_FALSE, "+001",     "%+04i", 1);

    TEST_SPRINTF(RT_FALSE, "0x1",      "%#x",   1);

    TEST_SPRINTF(RT_FALSE, "abcX",     "%2sX",  "abc");
    TEST_SPRINTF(RT_FALSE, "abcX",     "%-2sX", "abc");

    TEST_SPRINTF(RT_FALSE, "001234",   "%.6u",  1234);
    TEST_SPRINTF(RT_FALSE, "-001234",  "%.6i",  -1234);
    TEST_SPRINTF(RT_FALSE, "  1234",   "%6u",   1234);
    TEST_SPRINTF(RT_FALSE, " -1234",   "%6i",   -1234);
    TEST_SPRINTF(RT_FALSE, "001234",   "%06u",  1234);
    TEST_SPRINTF(RT_FALSE, "-01234",   "%06i",  -1234);
    TEST_SPRINTF(RT_FALSE, "1234  ",   "%-6u",  1234);

    TEST_SPRINTF(RT_FALSE, "-1234 ",   "%-6i",  -1234);
    TEST_SPRINTF(RT_FALSE, "1234",     "%.6s",  "1234");
    TEST_SPRINTF(RT_FALSE, "  1234",   "%6s",   "1234");
    TEST_SPRINTF(RT_FALSE, "1234  ",   "%-6s",  "1234");
    TEST_SPRINTF(RT_FALSE, " 01234",   "%6.5u", 1234);
    TEST_SPRINTF(RT_FALSE, "-01234",   "%6.5i", -1234);
    TEST_SPRINTF(RT_FALSE, "  1234",   "%6.5s", "1234");

    TEST_SPRINTF(RT_FALSE, "rt-thread", "%s",   "rt-thread");
}

static rt_err_t utest_tc_init(void)
{
    utest_log_lv_set(UTEST_LOG_ASSERT);
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    utest_log_lv_set(UTEST_LOG_ALL);
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_sprintf);
    UTEST_UNIT_RUN(test_rt_sprintf);
}
UTEST_TC_EXPORT(testcase, "testcases.kservice.sprintf_tc", utest_tc_init, utest_tc_cleanup, 60);
