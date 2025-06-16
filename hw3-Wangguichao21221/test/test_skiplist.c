#include "debug_util.h"
#include "skiplist.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

void testNode(SkipListNode *node) {
  if (node == NULL) {
    printf("Testing node: NULL\n");
    return;
  }
  printf("Testing node: %s, %d\n", node->member, node->score);
}
int main(void) {
  srand(time(NULL));

  SkipList *sl = NULL;
  // SkipListNode *node = NULL;
  // SkipListNode **result = NULL;
  // int count = 0;

  sl = sl_create();
  assert_true(sl != NULL);
  assert_int_eq(sl_get_length(sl), 0);

  sl_print(sl);
  
  // assert_int_eq(sl_insert(sl, "player1", 100), 1);
  // assert_int_eq(sl_insert(sl, "player2", 200), 1);
  // assert_int_eq(sl_insert(sl, "player3", 300), 1);
  // assert_int_eq(sl_insert(sl, "player4", 400), 1);
  // assert_int_eq(sl_insert(sl, "player5", 500), 1);
  // assert_int_eq(sl_get_length(sl), 5);
  sl_insert(sl, "player1", 100);
    sl_print(sl);
  sl_insert(sl, "player2", 200);
    sl_print(sl);
  sl_insert(sl, "player3", 300);
    sl_print(sl);
  sl_insert(sl, "player4", 400);
    sl_print(sl);
  sl_insert(sl, "player5", 500);
    sl_print(sl);
  sl_insert(sl, "player6", 100);
    sl_print(sl);
  sl_insert(sl, "player7", -600);
    sl_print(sl);
  sl_insert(sl, "player7", 0);
    sl_print(sl);
  assert_int_eq(sl_insert(sl, "player8", 100),0);
  sl_print(sl);
  printf("Start testing get by score\n");
  SkipListNode *textnode1 = sl_get_by_score(sl, -12);
  SkipListNode *textnode2 = sl_get_by_score(sl, 0);
  SkipListNode *textnode3 = sl_get_by_score(sl, 100);
  SkipListNode *textnode4 = sl_get_by_score(sl, 350);
  SkipListNode *textnode5 = sl_get_by_score(sl, 500);
  SkipListNode *textnode6 = sl_get_by_score(sl, 1000);
  SkipListNode *textnode7 = sl_get_by_score(sl, -600);

  testNode(textnode1);
  testNode(textnode2);
  testNode(textnode3);
  testNode(textnode4);
  testNode(textnode5);
  testNode(textnode6);
  testNode(textnode7);
  sl_remove(sl, 100);
  sl_remove(sl, 500);
  sl_remove(sl, 123);


  
  // assert_int_eq(sl_insert(sl, "player6", 300), 0);

  // node = sl_get_by_score(sl, 300);
  // assert_true(node != NULL);
  // assert_string_eq(node->member, "player3");
  // assert_int_eq(node->score, 300);

  // assert_int_eq(sl_get_rank_by_score(sl, 300), 3);

  // node = sl_get_by_rank(sl, 3);
  // assert_true(node != NULL);
  // assert_string_eq(node->member, "player3");

  // result = sl_get_range_by_rank(sl, 2, 4, &count);
  // assert_true(result != NULL);
  // assert_int_eq(count, 3);
  // assert_string_eq(result[0]->member, "player2");
  // assert_string_eq(result[1]->member, "player3");
  // assert_string_eq(result[2]->member, "player4");
  // free(result);

  // result = sl_get_range_by_score(sl, 200, 400, &count);
  // assert_true(result != NULL);
  // assert_int_eq(count, 3);
  // free(result);
  // sl_print(sl);
  // sl_remove(sl, 300);
  // // assert_int_eq(sl_remove(sl, 300), 1);
  // sl_print(sl);
  // assert_int_eq(sl_get_length(sl), 4);

  // assert_int_eq(sl_get_rank_by_score(sl, 400), 3);

  // assert_int_eq(sl_insert(sl, "player6", 600), 1);
  // assert_int_eq(sl_insert(sl, "player7", 300), 1);
  // sl_print(sl);

  // result = sl_get_range_by_score(sl, 200, 500, &count);
  // assert_true(result != NULL);
  // assert_int_eq(count, 4);
  // free(result);
  // printf("before free!\n");

  sl_free(sl);
  printf("All tests passed!\n");
  return 0;
}
