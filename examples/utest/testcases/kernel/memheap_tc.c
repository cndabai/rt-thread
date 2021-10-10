/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     flybreak     the first version
 */

#include <rtthread.h>
#include <stdlib.h>
#include "utest.h"

#define HEAP_SIZE        (64 * 1024)
#define HEAP_ALIGN       (4)
#define SLICE_NUM        (40)
#define TEST_TIMES       (100000)
#define HEAP_NAME        "heap1"
#define SLICE_SIZE_MAX   (HEAP_SIZE/SLICE_NUM)

static void memheap_test(void)
{
    struct rt_memheap heap1;
    rt_uint32_t ptr_start;
    void *ptr[SLICE_NUM];
    int i, cnt = 0;

    /* init heap */
    ptr_start = (rt_uint32_t)rt_malloc_align(HEAP_SIZE, HEAP_ALIGN);
    if (ptr_start == RT_NULL)
    {
        rt_kprintf("totle size too big,can not malloc memory!");
        return;
    }

    rt_memheap_init(&heap1, HEAP_NAME, (void *)ptr_start, HEAP_SIZE);

    /* test start */
    for (i = 0; i < SLICE_NUM; i++)
    {
        ptr[i] = 0;
    }
    /* test alloc */
    for (i = 0; i < SLICE_NUM; i++)
    {
        rt_uint32_t slice_size = rand() % SLICE_SIZE_MAX;
        ptr[i] = rt_memheap_alloc(&heap1, slice_size);
    }
    /* test realloc */
    while (cnt < TEST_TIMES)
    {
        rt_uint32_t slice_size = rand() % SLICE_SIZE_MAX;
        rt_uint32_t ptr_index = rand() % SLICE_NUM;
        rt_uint32_t operation = rand() % 2;

        if (ptr[ptr_index])
        {
            if (operation == 0) /* free and malloc */
            {
                if (ptr[ptr_index])
                {
                    rt_memheap_free(ptr[ptr_index]);
                }
                ptr[ptr_index] = rt_memheap_alloc(&heap1, slice_size);
            }
            else /* realloc */
            {
                ptr[ptr_index] = rt_memheap_realloc(&heap1, ptr[ptr_index], slice_size);
            }
        }
        cnt ++;
        if (cnt % (TEST_TIMES / 10) == 0)
        {
            rt_kprintf(">");
        }
    }

    rt_kprintf("test OK!\n");

    /* test end */
    rt_memheap_detach(&heap1);
    rt_free_align((void *)ptr_start);
}

static void test_rt_malloc_free(void)
{
    char *ptr1 = RT_NULL;
    char *ptr2 = RT_NULL;
    int i;

#ifndef RT_USING_MEMHEAP_AS_HEAP
    if (rt_malloc(0) != RT_NULL)
    {
        LOG_E("rt_malloc failed");
        uassert_false(1);
        return;
    }
#endif

    ptr1 = rt_malloc(16);
    if (ptr1 == RT_NULL)
    {
        LOG_E("rt_malloc failed");
        uassert_false(ptr1 == RT_NULL);
        goto __exit;
    }
    for (i = 0; i < 16; i ++)
    {
        *ptr1 ++ = i;
    }
    ptr1 -= 16;
    for (i = 0; i < 16; i ++)
    {
        if (*ptr1 ++ != i)
        {
            LOG_E("rt_malloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr1 -= 16;
    
    ptr2 = rt_malloc(17);
    if (ptr2 == RT_NULL)
    {
        LOG_E("rt_malloc failed");
        goto __exit;
    }
    for (i = 0; i < 17; i ++)
    {
        *ptr2 ++ = i;
    }
    ptr2 -= 17;
    for (i = 0; i < 17; i ++)
    {
        if (*ptr2 ++ != i)
        {
            LOG_E("rt_malloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr2 -= 17;
    uassert_true(i == 17);

__exit:
    if (ptr1 != RT_NULL)
    {
        rt_free(ptr1);
    }
    if (ptr2 != RT_NULL)
    {
        rt_free(ptr2);
    }
    return;
}

static void test_rt_realloc_more(void)
{
    char *ptr = RT_NULL;
    char *ptr_realloc_more1 = RT_NULL;
    char *ptr_realloc_more2 = RT_NULL;
    char *reptr_realloc_more = RT_NULL;
    int i;

    ptr = rt_malloc(16);
    if (ptr == RT_NULL)
    {
        LOG_E("rt_malloc failed");
        goto __exit;
    }
    for (i = 0; i < 16; i ++)
    {
        *ptr ++ = i;
    }
    ptr -= 16;

    ptr_realloc_more1 = rt_realloc(ptr, 32);
    reptr_realloc_more = ptr_realloc_more1;
    if (ptr_realloc_more1 == RT_NULL)
    {
        LOG_E("rt_realloc failed!");
        goto __exit;
    }
    for (i = 0; i < 16; i ++)
    {
        if (*ptr_realloc_more1 ++ != i)
        {
            LOG_E("rt_realloc failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    for (i = 16; i < 32; i ++)
    {
        *ptr_realloc_more1 ++ = i;
    }
    ptr_realloc_more1 -= 32;
    for (i = 0; i < 32; i ++)
    {
        if (*ptr_realloc_more1 ++ != i)
        {
            LOG_E("rt_realloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr_realloc_more1 -= 32;

    ptr_realloc_more2 = rt_realloc(ptr_realloc_more1, 37);
    reptr_realloc_more = ptr_realloc_more2;
    if (ptr_realloc_more2 == RT_NULL)
    {
        LOG_E("rt_realloc failed!");
        goto __exit;
    }
    ptr_realloc_more2 += 32;
    for (i = 32; i < 37; i ++)
    {
        *ptr_realloc_more2 ++ = i;
    }
    ptr_realloc_more2 -= 37;
    for (i = 0; i < 37; i ++)
    {
        if (*ptr_realloc_more2 ++ != i)
        {
            LOG_E("rt_realloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    uassert_true(i == 37);

__exit:
    if (reptr_realloc_more != RT_NULL)
    {
        rt_free(reptr_realloc_more);
        reptr_realloc_more = RT_NULL;
    }
    return;
}

static void test_rt_realloc_less(void)
{
    char *ptr = RT_NULL;
    char *ptr_realloc_less1 = RT_NULL;
    char *ptr_realloc_less2 = RT_NULL;
    char *reptr_realloc_less = RT_NULL;
    int i;

    ptr = rt_malloc(16);
    if (ptr == RT_NULL)
    {
        LOG_E("rt_malloc failed");
        uassert_false(ptr == RT_NULL);
        goto __exit;
    }
    for (i = 0; i < 16; i ++)
    {
        *ptr ++ = i;
    }
    ptr -= 16;

    ptr_realloc_less1 = rt_realloc(ptr, 8);
    reptr_realloc_less = ptr_realloc_less1;
    if (ptr_realloc_less1 == RT_NULL)
    {
        LOG_E("rt_realloc failed!");
        uassert_false(ptr_realloc_less1 == RT_NULL);
        goto __exit;
    }
    for (i = 0; i < 8; i ++)
    {
        if (*ptr_realloc_less1 ++ != i)
        {
            LOG_E("rt_realloc failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr_realloc_less1 -= 8;

    ptr_realloc_less2 = rt_realloc(ptr_realloc_less1, 5);
    reptr_realloc_less = ptr_realloc_less2;
    for (i = 0; i < 5; i ++)
    {
        if (*ptr_realloc_less2 ++ != i)
        {
            LOG_E("rt_realloc failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    uassert_true(i == 5);

__exit:
    if (reptr_realloc_less != RT_NULL)
    {
        rt_free(reptr_realloc_less);
        reptr_realloc_less = RT_NULL;
    }
    return;
}

static void test_rt_calloc(void)
{
    char *ptr1 = RT_NULL;
    char *ptr2 = RT_NULL;
    int i;

    ptr1 = rt_calloc(4, 16);
    if (ptr1 == RT_NULL)
    {
        LOG_E("rt_calloc failed");
        uassert_false(ptr1 == RT_NULL);
        goto __exit;
    }
    for (i = 0; i < (4 * 16); i ++)
    {
        *ptr1 ++ = i;
    }
    ptr1 -= (4 * 16);
    for (i = 0; i < (4 * 16); i ++)
    {
        if(*ptr1 ++ != i)
        {
            LOG_E("rt_calloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr1 -= (4 * 16);

    ptr2 = rt_calloc(3, 5);
    if (ptr2 == RT_NULL)
    {
        LOG_E("rt_calloc failed");
        uassert_false(ptr2 == RT_NULL);
        goto __exit;
    }
    for (i = 0; i < (3 * 5); i ++)
    {
        *ptr2 ++ = i;
    }
    ptr2 -= (3 * 5);
    for (i = 0; i < (3 * 5); i ++)
    {
        if(*ptr2 ++ != i)
        {
            LOG_E("rt_calloc failed");
            uassert_false(1);
            goto __exit;
        }
    }
    ptr2 -= (3 * 5);
    uassert_true(i == (3 * 5));

__exit:
    if(ptr1 != RT_NULL)
    {
        rt_free(ptr1);
    }
    if(ptr2 != RT_NULL)
    {
        rt_free(ptr2);
    }
    return;
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(memheap_test);
    UTEST_UNIT_RUN(test_rt_malloc_free);
    UTEST_UNIT_RUN(test_rt_realloc_more);
    UTEST_UNIT_RUN(test_rt_realloc_less);
    UTEST_UNIT_RUN(test_rt_calloc);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.memheap_tc", utest_tc_init, utest_tc_cleanup, 10);
