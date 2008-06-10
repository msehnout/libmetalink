#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
/* include test cases' include files here */
#include "list_test.h"
#include "metalink_pctrl_test.h"
#include "metalink_parser_test.h"

int init_suite1(void)
{
  return 0;
}

int clean_suite1(void)
{
  return 0;
}


int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("libmetalink_TestSuite", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if((!CU_add_test(pSuite, "test of list", test_list))
      ||
      (!CU_add_test(pSuite, "test of metalink_pctrl_file_transaction",
		    test_metalink_pctrl_file_transaction))
      ||
      (!CU_add_test(pSuite, "test of metalink_pctrl_resource_transaction",
		    test_metalink_pctrl_resource_transaction))
      ||
      (!CU_add_test(pSuite, "test of metalink_pctrl_checksum_transaction",
		    test_metalink_pctrl_checksum_transaction))
      ||
      (!CU_add_test(pSuite, "test of metalink_pctrl_piece_hash_transaction",
		    test_metalink_pctrl_piece_hash_transaction))
      ||
      (!CU_add_test(pSuite, "test of metalink_pctrl_chunk_checksum_transaction",
		    test_metalink_pctrl_chunk_checksum_transaction))
      ||
      (!CU_add_test(pSuite, "test of metalink_parser_test1_xml",
		    test_metalink_parser_test1_xml))
      ) {
     CU_cleanup_registry();
     return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}