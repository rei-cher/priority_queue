#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

// TODO: Incorporate into cmake test build so it builds automatically
#include "../include/queue_p.h"

// keeps track of how many times custom free is actually called
static int g_custom_free_calls = 0;

static void test_custom_free(void *mem_addr)
{
    g_custom_free_calls++;
    free(mem_addr);
    mem_addr = NULL;
}

// will help simplify pointer checking
static void *check_calloc(size_t num)
{
    void *pointer = calloc(1, num);
    //Fatal so as to not continue tests beyond a failed calloc
    CU_ASSERT_PTR_NOT_NULL_FATAL(pointer);
    return pointer;
}

static void test_queue_p_init_valid_customfree(void)
{
    queue_p_t *queue = queue_p_init(4, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL(queue);
    if (queue)
    {
        CU_ASSERT_EQUAL(queue->capacity, 4);
        CU_ASSERT_EQUAL(queue->currentsz, 0);
        CU_ASSERT_PTR_NOT_NULL(queue->arr);
        CU_ASSERT_PTR_EQUAL(queue->customfree, test_custom_free);

        CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
        CU_ASSERT_PTR_NULL(queue);
    }
}

static void test_queue_p_init_null_customfree_still_works(void)
{
    queue_p_t *queue = queue_p_init(2, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    int *arb_data = (int *)check_calloc(sizeof(int));
    *arb_data = 42;

    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data, 1), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_emptycheck(queue), 0);

    queue_p_node_t *node = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node->data);
    CU_ASSERT_EQUAL(*(int *)node->data, 42);

    free(node->data);
    node->data = NULL;
    free(node);
    node = NULL;

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);
}

static void test_queue_p_empty_and_full_checks(void)
{
    queue_p_t *queue = queue_p_init(2, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    CU_ASSERT_EQUAL(queue_p_emptycheck(queue), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_fullcheck(queue), 0);

    int *arb_data_one = (int *)check_calloc(sizeof(int));
    int *arb_data_two = (int *)check_calloc(sizeof(int));
    *arb_data_one = 1;
    *arb_data_two = 2;

    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_one, 5), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_emptycheck(queue), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_fullcheck(queue), 0);

    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_two, 6), 0);
    CU_ASSERT_EQUAL(queue_p_fullcheck(queue), 0);

    int *arb_data_3 = (int *)check_calloc(sizeof(int));
    *arb_data_3 = 3;
    CU_ASSERT_NOT_EQUAL(queue_p_enqueue(queue, arb_data_3, 999), 0);
    free(arb_data_3);
    arb_data_3 = NULL;

    g_custom_free_calls = 0;
    CU_ASSERT_EQUAL(queue_p_clear(queue), 0);
    CU_ASSERT_EQUAL(g_custom_free_calls, 2);
    CU_ASSERT_EQUAL(queue_p_emptycheck(queue), 0);

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);
}

static void test_queue_p_peek_does_not_remove(void)
{
    queue_p_t *queue = queue_p_init(3, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    int *arb_data_a = (int *)check_calloc(sizeof(int));
    int *arb_data_b = (int *)check_calloc(sizeof(int));
    *arb_data_a = 100;
    *arb_data_b = 200;

    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_a, 1), 0);
    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_b, 2), 0);

    queue_p_node_t *peek_node = queue_p_peek(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(peek_node);
    CU_ASSERT_PTR_NOT_NULL_FATAL(peek_node->data);
    CU_ASSERT_EQUAL(*(int *)peek_node->data, 200);
    CU_ASSERT_EQUAL(peek_node->priority, 2);

    queue_p_node_t *node1 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node1);
    CU_ASSERT_EQUAL(*(int *)node1->data, 200);
    CU_ASSERT_EQUAL(node1->priority, 2);

    test_custom_free(node1->data);
    free(node1);
    node1 = NULL;

    queue_p_node_t *node2 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node2);
    CU_ASSERT_EQUAL(*(int *)node2->data, 100);
    CU_ASSERT_EQUAL(node2->priority, 1);

    test_custom_free(node2->data);
    free(node2);
    node2 = NULL;

    CU_ASSERT_EQUAL(queue_p_emptycheck(queue), 0);

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);
}

static void test_queue_p_priority_ordering_highest_first(void)
{
    queue_p_t *queue = queue_p_init(10, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    int *arb_data_a = (int *)check_calloc(sizeof(int));
    *arb_data_a = 1;
    int *arb_data_b = (int *)check_calloc(sizeof(int));
    *arb_data_b = 2;
    int *arb_data_c = (int *)check_calloc(sizeof(int));
    *arb_data_c = 3;
    int *arb_data_d = (int *)check_calloc(sizeof(int));
    *arb_data_d = 4;

    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_a, 10), 0);
    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_b, 50), 0);
    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_c, 20), 0);
    CU_ASSERT_EQUAL(queue_p_enqueue(queue, arb_data_d, 50), 0);

    queue_p_node_t *node1 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node1);
    CU_ASSERT_EQUAL(node1->priority, 50);
    CU_ASSERT(*(int *)node1->data == 2 || *(int *)node1->data == 4);
    test_custom_free(node1->data);
    free(node1);
    node1 = NULL;

    queue_p_node_t *node2 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node2);
    CU_ASSERT_EQUAL(node2->priority, 50);
    CU_ASSERT(*(int *)node2->data == 2 || *(int *)node2->data == 4);
    test_custom_free(node2->data);
    free(node2);
    node2 = NULL;

    queue_p_node_t *node3 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node3);
    CU_ASSERT_EQUAL(node3->priority, 20);
    CU_ASSERT_EQUAL(*(int *)node3->data, 3);
    test_custom_free(node3->data);
    free(node3);
    node3 = NULL;

    queue_p_node_t *node4 = queue_p_dequeue(queue);
    CU_ASSERT_PTR_NOT_NULL_FATAL(node4);
    CU_ASSERT_EQUAL(node4->priority, 10);
    CU_ASSERT_EQUAL(*(int *)node4->data, 1);
    test_custom_free(node4->data);
    free(node4);
    node4 = NULL;

    CU_ASSERT_PTR_NULL(queue_p_dequeue(queue));
    CU_ASSERT_PTR_NULL(queue_p_peek(queue));

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);
}

static void test_queue_p_clear_frees_all_items(void)
{
    queue_p_t *queue = queue_p_init(5, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    g_custom_free_calls = 0;

    for (int idx = 0; idx < 5; idx++)
    {
        int *data = (int *)check_calloc(sizeof(int));
        *data = idx;
        CU_ASSERT_EQUAL(queue_p_enqueue(queue, data, idx), 0);
    }

    CU_ASSERT_EQUAL(queue_p_fullcheck(queue), 0);

    CU_ASSERT_EQUAL(queue_p_clear(queue), 0);
    CU_ASSERT_EQUAL(g_custom_free_calls, 5);
    CU_ASSERT_EQUAL(queue_p_emptycheck(queue), 0);

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);
}

static void test_queue_p_destroy_on_nonempty_frees_items(void)
{
    queue_p_t *queue = queue_p_init(3, test_custom_free);
    CU_ASSERT_PTR_NOT_NULL_FATAL(queue);

    g_custom_free_calls = 0;

    for (int idx = 0; idx < 3; idx++)
    {
        char *string = (char *)check_calloc(16);
        snprintf(string, 16, "item%d", idx);
        CU_ASSERT_EQUAL(queue_p_enqueue(queue, string, 100 - idx), 0);
    }

    CU_ASSERT_EQUAL(queue_p_destroy(&queue), 0);
    CU_ASSERT_PTR_NULL(queue);

    CU_ASSERT_EQUAL(g_custom_free_calls, 3);
}

// Pass NULL pointers into everything
static void test_queue_p_invalid_args_defensive(void)
{
    CU_ASSERT_NOT_EQUAL(queue_p_fullcheck(NULL), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_emptycheck(NULL), 0);
    CU_ASSERT_NOT_EQUAL(queue_p_enqueue(NULL, (void *)0x1, 1), 0);
    CU_ASSERT_PTR_NULL(queue_p_dequeue(NULL));
    CU_ASSERT_PTR_NULL(queue_p_peek(NULL));
    CU_ASSERT_NOT_EQUAL(queue_p_destroy(NULL), 0);

    queue_p_t *dead_queue = NULL;
    CU_ASSERT_NOT_EQUAL(queue_p_destroy(&dead_queue), 0);
}

int main(void)
{
    // https://cunit.sourceforge.net/doc/test_registry.html
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("queue_p_tests", NULL, NULL);
    if (!suite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_ADD_TEST(suite, test_queue_p_init_valid_customfree);
    CU_ADD_TEST(suite, test_queue_p_init_null_customfree_still_works);
    CU_ADD_TEST(suite, test_queue_p_empty_and_full_checks);
    CU_ADD_TEST(suite, test_queue_p_peek_does_not_remove);
    CU_ADD_TEST(suite, test_queue_p_priority_ordering_highest_first);
    CU_ADD_TEST(suite, test_queue_p_clear_frees_all_items);
    CU_ADD_TEST(suite, test_queue_p_destroy_on_nonempty_frees_items);
    CU_ADD_TEST(suite, test_queue_p_invalid_args_defensive);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    unsigned int failures = CU_get_number_of_failures();
    CU_cleanup_registry();
    return (failures == 0) ? 0 : 1;
}
