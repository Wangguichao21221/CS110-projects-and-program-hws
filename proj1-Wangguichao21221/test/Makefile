VALGRIND = valgrind --tool=memcheck --leak-check=full --track-origins=yes
FULL_TESTS = labels full_inst simple1 p1_errors p2_errors

.PHONY: clean check test

all: check

clean:
	rm -rf out

make_out_dirs:
	@-mkdir -p out

check: make_out_dirs
	@echo "Running tests..."
	@$(foreach test, $(FULL_TESTS), \
		echo "Testing $(test)..."; \
		$(VALGRIND) --log-file=out/$(test).memcheck ../assembler --input_file in/$(test).s --output_folder out/; \
		DIFF_LOG_FAIL=0; DIFF_OUT_FAIL=0; VALGRIND_FAIL=0; \
		if ! diff out/$(test).log ref/$(test).log > /dev/null 2>&1; then DIFF_LOG_FAIL=1; fi; \
		if ! diff out/$(test).out ref/$(test).out > /dev/null 2>&1; then DIFF_OUT_FAIL=1; fi; \
		if ! grep -q "ERROR SUMMARY: 0 errors" out/$(test).memcheck; then VALGRIND_FAIL=1; fi; \
		if [ $${DIFF_LOG_FAIL} -eq 0 ] && [ $${DIFF_OUT_FAIL} -eq 0 ] && [ $${VALGRIND_FAIL} -eq 0 ]; then \
			echo "PASS"; \
		else \
			if [ $${DIFF_OUT_FAIL} -ne 0 ]; then echo "Diff .out check failed"; fi; \
			if [ $${DIFF_LOG_FAIL} -ne 0 ]; then echo "Diff .log check failed"; fi; \
			if [ $${VALGRIND_FAIL} -ne 0 ]; then echo "Valgrind check failed"; fi; \
		fi; \
	)
	

test: make_out_dirs
	@echo "Running single test: $(TEST_NAME)"
	@$(VALGRIND) --log-file=out/$(TEST_NAME).memcheck ../assembler --input_file in/$(TEST_NAME).s --output_folder out/
	@DIFF_OUT_FAIL=0; DIFF_LOG_FAIL=0; VALGRIND_FAIL=0; \
	if ! diff out/$(TEST_NAME).log ref/$(TEST_NAME).log > /dev/null 2>&1; then DIFF_LOG_FAIL=1; fi; \
	if ! diff out/$(TEST_NAME).out ref/$(TEST_NAME).out > /dev/null 2>&1; then DIFF_OUT_FAIL=1; fi; \
	if ! grep -q "ERROR SUMMARY: 0 errors" out/$(TEST_NAME).memcheck; then VALGRIND_FAIL=1; fi; \
	if [ $${DIFF_LOG_FAIL} -eq 0 ] && [ $${DIFF_OUT_FAIL} -eq 0 ] && [ $${VALGRIND_FAIL} -eq 0 ]; then \
		echo "PASS"; \
	else \
		if [ $${DIFF_OUT_FAIL} -ne 0 ]; then echo "Diff .out check failed"; fi; \
		if [ $${DIFF_LOG_FAIL} -ne 0 ]; then echo "Diff .log check failed"; fi; \
		if [ $${VALGRIND_FAIL} -ne 0 ]; then echo "Valgrind check failed"; fi; \
	fi

TEST_NAME ?= 