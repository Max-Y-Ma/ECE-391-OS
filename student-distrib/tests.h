#ifndef TESTS_H
#define TESTS_H

/* Test Macros */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

/* Test Launcher */
void launch_tests();

/* Tests */
int idt_test(void);
int paging_test(void);
int keyboard_raw_test(void);
int terminal_test(void);

#endif /* TESTS_H */
