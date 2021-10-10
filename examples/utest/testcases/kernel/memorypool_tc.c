/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identif ier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-04-16     WillianChan     the first version
 * 2019-11-28     WillianChan     cancel the test that the three
 *                                parameters of size, block_count,
 *                                block_size in the memorypool are 0
 */

#include <rtthread.h>
#include "utest.h"

#define MP_STATIC  (1)
#define MP_DYNAMIC (0)

#define TEST_MP_SIZE         1024
#define TEST_BLOCK_SIZE      TEST_MP_SIZE / sizeof(rt_uint8_t *)

static rt_uint8_t *mp_block[TEST_BLOCK_SIZE] = {0};
static rt_uint8_t  mempool[TEST_MP_SIZE];
static rt_thread_t mp_thread1, mp_thread2;
static rt_sem_t mp_sem;
static struct rt_mempool test_mp1;
static rt_mp_t test_mp2;

static rt_size_t block_size_test;
static rt_err_t mp_rst = RT_EOK;
static rt_bool_t mp_alloc_finish, mp_free_finish, mp_excp, mp_suspend_flag;

static rt_mp_t mp_static_dynamic(rt_size_t size,
                                 rt_size_t block_size,
                                 rt_size_t block_count,
                                 rt_bool_t flag)
{
    rt_mp_t mp;

    rt_memset(mp_block, RT_NULL, TEST_BLOCK_SIZE);
    if (flag == MP_STATIC)
    {
        if (rt_mp_init(&test_mp1, "test_static_mp", &mempool[0], size, block_size) != RT_EOK)
        {
            LOG_E("Init memorypool failed.\n");
            return RT_NULL;
        }

        size = RT_ALIGN_DOWN(size, RT_ALIGN_SIZE);
        block_size = RT_ALIGN(block_size, RT_ALIGN_SIZE);
        if (test_mp1.start_address != mempool ||
            test_mp1.size != size ||
            test_mp1.block_size != block_size ||
            test_mp1.block_total_count != size / (block_size + sizeof(rt_uint8_t *)) ||
            test_mp1.block_free_count != size / (block_size + sizeof(rt_uint8_t *)))
        {
            LOG_E("Init memorypool failed.\n");
            rt_mp_detach(&test_mp1);
            return RT_NULL;
        }
        mp = &test_mp1;
    }
    else
    {
        test_mp2 = rt_mp_create("test_mp2", block_count, block_size);
        if (test_mp2 == RT_NULL)
        {
            LOG_E("Create Memorypool failed.\n");
            return RT_NULL;
        }
        block_size = RT_ALIGN_DOWN(block_size, RT_ALIGN_SIZE);
        if (test_mp2->size != (block_size + sizeof(rt_uint8_t *)) * block_count ||
                test_mp2->block_total_count != block_count)
        {
            LOG_E("Create Memorypool failed.\n");
            rt_mp_delete(test_mp2);
            return RT_NULL;
        }
        mp = test_mp2;
    }
    block_size_test = mp->block_size;
    return mp;
}

static void mp_cleanup(rt_bool_t mp_flag)
{
    mp_thread1 = RT_NULL;
    mp_thread2 = RT_NULL;
    if (mp_sem != RT_NULL)
    {
        rt_sem_delete(mp_sem);
    }
    if (mp_flag)
    {
        rt_mp_detach(&test_mp1);
    }
    else
    {
        rt_mp_delete(test_mp2);
    }
    mp_sem = RT_NULL;
    test_mp2 = RT_NULL;
    block_size_test = 0;
    mp_rst = RT_EOK;
    mp_alloc_finish = RT_FALSE;
    mp_free_finish = RT_FALSE;
    mp_excp = RT_FALSE;
    mp_suspend_flag = RT_FALSE;
}

static rt_err_t mp_free_all_test(void)
{
    int i;

    for (i = 0; i < TEST_BLOCK_SIZE; i++)
    {
        if (mp_block[i] == RT_NULL)
        {
            continue;
        }
        rt_mp_free(mp_block[i]);
        mp_block[i] = RT_NULL;
    }

    return RT_EOK;
}

static rt_err_t mp_alloc_all_test(rt_mp_t mp)
{
    rt_size_t alloc_num = 0;

    /* exclude the allocated memory block */
    while (mp_block[alloc_num] != RT_NULL)
    {
        alloc_num++;
    }

    /* allocate all blocks from memory pool */
    while (mp->block_free_count)
    {
        mp_block[alloc_num] = rt_mp_alloc(mp, RT_WAITING_FOREVER);
        if (mp_block[alloc_num] == RT_NULL)
        {
            LOG_E("mp_alloc_all_test: The memorypool block is not RT_NULL.\n");
            mp_free_all_test();
            return -RT_ERROR;
        }
        alloc_num++;
    }

    return RT_EOK;
}

static rt_err_t mp_rw_test(rt_mp_t mp)
{
    rt_size_t alloc_num = 0;
    rt_uint32_t i;

    /* Write data to each allocated memory blocks */
    while (mp_block[alloc_num] != RT_NULL && alloc_num < TEST_BLOCK_SIZE)
    {
        rt_memset(mp_block[alloc_num], alloc_num, mp->block_size);
        for (i = 0; i < mp->block_size; i++)
        {
            if (alloc_num != *(mp_block[alloc_num] + i))
                break;
        }
        if (i != mp->block_size)
        {
            LOG_E("Write/Read message failed.\n");
            mp_free_all_test();
            return -RT_ERROR;
        }
        alloc_num++;
    }

    return RT_EOK;
}

static rt_err_t test_mp_rw(rt_size_t mp_size,
                           rt_size_t mp_block_size,
                           rt_size_t mp_block_count,
                           rt_bool_t mp_flag)
{
    rt_mp_t mp;

    mp = mp_static_dynamic(mp_size, mp_block_size, mp_block_count, mp_flag);
    if (mp == RT_NULL)
    {
        if ((mp_block_count == 0) && (mp_flag == MP_DYNAMIC))
        {
            return RT_EOK;
        }
        return -RT_ERROR;
    }
    if (mp_alloc_all_test(mp) != RT_EOK || mp->block_free_count != 0)
    {
        LOG_E("Alloc memory pool failed.\n");
        mp_excp = RT_TRUE;
        goto __EXIT;
    }
    
    if (mp_rw_test(mp) != RT_EOK)
    {
        LOG_E("Read memory block number wrong.\n");
        mp_excp = RT_TRUE;
        goto __EXIT;
    }
    mp_free_all_test();

__EXIT:
    mp_cleanup(mp_flag);
    if (mp_excp)
    {
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static void thread1_entry(void *mp)
{
    rt_uint8_t *block, *mp_block_start;
    rt_uint8_t mp_count = 0;

    mp_suspend_flag = 1;
    mp_rst = mp_alloc_all_test(mp);
    uassert_true(mp_rst == RT_EOK);
    if (mp_rst != RT_EOK)
    {
        mp_excp = RT_TRUE;
        rt_sem_release(mp_sem);
        mp_alloc_finish = RT_TRUE;
        return;
    }
    rt_sem_release(mp_sem);

    /*
     * allocate a block from the memory pool,
     * if thread status is not suspended, it will masked.
     */
    block = rt_mp_alloc(mp, RT_WAITING_FOREVER);
    mp_suspend_flag = 0;
    uassert_true(block != RT_NULL);
    rt_sem_take(mp_sem, RT_WAITING_FOREVER);
    if (block == RT_NULL || mp_block[0] != RT_NULL)
    {
        LOG_E("Alloc block failed.\n");
    }
    else
    {
        mp_block_start = block;
        for (mp_count = 0; (block - mp_block_start) < block_size_test; mp_count++, block++)
        {
            *block = mp_count;
        }
        block = mp_block_start;
        for (mp_count = 0; (block - mp_block_start) < block_size_test; mp_count++, block++)
        {
            if (*block != mp_count)
            {
                mp_excp = RT_TRUE;
                break;
            }
        }
        uassert_true((block - mp_block_start) == block_size_test);
        block = mp_block_start;
        rt_mp_free(block);
    }
    mp_alloc_finish = RT_TRUE;
}

static void thread2_entry(void *mp)
{
    rt_sem_take(mp_sem, RT_WAITING_FOREVER);
    if (mp_excp == RT_TRUE)
    {
        mp_free_finish = RT_TRUE;
        return;
    }
    if (mp_suspend_flag)
    {
        rt_mp_free(mp_block[0]);
        mp_block[0] = RT_NULL;
    }
    rt_sem_release(mp_sem);
    mp_free_all_test();
    mp_free_finish = RT_TRUE;
}

static rt_err_t test_mp_suspend(rt_size_t mp_size,
                                rt_size_t mp_block_size,
                                rt_size_t mp_block_count,
                                rt_bool_t mp_flag)
{
    rt_mp_t mp;

    mp_alloc_finish = RT_FALSE;
    mp_free_finish = RT_FALSE;
    mp_excp = RT_FALSE;

    mp = mp_static_dynamic(mp_size, mp_block_size, mp_block_count, mp_flag);
    if (mp == RT_NULL)
    {
        return -RT_ERROR;
    }
    mp_sem = rt_sem_create("mp_sem", 0, RT_IPC_FLAG_FIFO);
    if (mp_sem == RT_NULL)
    {
        LOG_E("Create semaphore failed.\n");
        mp_excp = RT_TRUE;
        goto __EXIT;
    }

    mp_thread1 = rt_thread_create("thread1",
                                  thread1_entry, mp,
                                  1024,
                                  15,
                                  10);
    mp_thread2 = rt_thread_create("thread2",
                                  thread2_entry, mp,
                                  1024,
                                  16,
                                  10);
    if (mp_thread1 != RT_NULL && mp_thread2 != RT_NULL)
    {
        rt_thread_startup(mp_thread1);
        rt_thread_startup(mp_thread2);
    }
    else
    {
        LOG_E("Create thread failed.\n");
        mp_excp = RT_TRUE;
        goto __EXIT;
    }
    while (!mp_alloc_finish || !mp_free_finish)
    {
        rt_thread_mdelay(1000);
    }

__EXIT:
    mp_cleanup(mp_flag);
    if (mp_excp)
    {
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static void test_static_rw(void)
{
    uassert_true(test_mp_rw(1024, 28, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_rw(1024, 27, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_rw(1023, 28, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_rw(1023, 27, RT_NULL, MP_STATIC) == RT_EOK);
}

static void test_dynamic_rw(void)
{
    uassert_true(test_mp_rw(RT_NULL, 28, 32, MP_DYNAMIC) == RT_EOK);
    uassert_true(test_mp_rw(RT_NULL, 28, 31, MP_DYNAMIC) == RT_EOK);
}

static void test_static_suspend(void)
{
    uassert_true(test_mp_suspend(1024, 28, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_suspend(1024, 27, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_suspend(1023, 28, RT_NULL, MP_STATIC) == RT_EOK);
    uassert_true(test_mp_suspend(1023, 27, RT_NULL, MP_STATIC) == RT_EOK);
}

static void test_dynamic_suspend(void)
{
    uassert_true(test_mp_suspend(RT_NULL, 28, 32, MP_DYNAMIC) == RT_EOK);
    uassert_true(test_mp_suspend(RT_NULL, 28, 31, MP_DYNAMIC) == RT_EOK);
}

static rt_err_t utest_tc_init(void)
{
    mp_thread1 = RT_NULL;
    mp_thread2 = RT_NULL;
    mp_sem = RT_NULL;
    test_mp2 = RT_NULL;
    block_size_test = 0;
    mp_rst = RT_EOK;
    mp_alloc_finish = RT_FALSE;
    mp_free_finish = RT_FALSE;
    mp_excp = RT_FALSE;
    mp_suspend_flag = RT_FALSE;

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_static_rw);
    UTEST_UNIT_RUN(test_dynamic_rw);
    UTEST_UNIT_RUN(test_static_suspend);
    UTEST_UNIT_RUN(test_dynamic_suspend);
}
UTEST_TC_EXPORT(testcase, "src.memory.memorypool_tc", utest_tc_init, utest_tc_cleanup, 10);
