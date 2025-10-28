CC = gcc
AR = ar rcs
CFLAGS = -Wall -Wextra -Werror -std=c11 -g
GCOV_FLAGS = -fprofile-arcs -ftest-coverage

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	TEST_FLAGS = -lcheck -lm -pthread
	OPEN_CMD = open
	GCOV_CMD = gcov
else
	TEST_FLAGS = -lcheck -lm -pthread
	OPEN_CMD = xdg-open
	GCOV_CMD = gcov
endif

SOURCES = utils.c arithmetic.c compare.c
OBJECTS = $(SOURCES:.c=.o)

TEST_SOURCES = test_decimal.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

LIBRARY = decimal.a
TEST_EXEC = test
TEST_EXEC_GCOV = $(TEST_EXEC)_gcov

.PHONY: all clean test gcov_report valgrind leaks clang

all: $(LIBRARY)

%.o: %.c decimal.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	$(AR) $@ $^

test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJECTS) $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(TEST_FLAGS)

$(TEST_EXEC_GCOV): $(SOURCES) $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(GCOV_FLAGS) $^ -o $@ $(TEST_FLAGS)

clang:
	cp ../materials/linters/.clang-format .
	clang-format -n *.c *.h
	rm ./.clang-format

leaks: $(TEST_EXEC) $(TEST_EXEC_GCOV)
	@echo "Memory leak check with leaks (regular test)"
	leaks -atExit -- ./$(TEST_EXEC) || true
	@echo "Memory leak check with leaks (gcov test)"
	leaks -atExit -- ./$(TEST_EXEC_GCOV) || true

valgrind: $(TEST_EXEC) $(TEST_EXEC_GCOV)
	@echo "Memory leak check with valgrind (regular test)"
	valgrind --log-file=valgrind_test.log --leak-check=full --show-leak-kinds=all ./$(TEST_EXEC) || true
	@echo "Memory leak check with valgrind (gcov test)"
	valgrind --log-file=valgrind_gcov.log --leak-check=full --show-leak-kinds=all ./$(TEST_EXEC_GCOV) || true

gcov_report: $(TEST_EXEC_GCOV)
	@echo "Running tests with coverage flags"
	-./$(TEST_EXEC_GCOV)
	@echo "Creating coverage report"
	for file in $(SOURCES); do \
		base_name=$$(basename $$file .c); \
		if [ -f $(TEST_EXEC_GCOV)-$$base_name.gcno ]; then \
			cp $(TEST_EXEC_GCOV)-$$base_name.gcno $$base_name.gcno; \
		fi; \
		if [ -f $(TEST_EXEC_GCOV)-$$base_name.gcda ]; then \
			cp $(TEST_EXEC_GCOV)-$$base_name.gcda $$base_name.gcda; \
		fi; \
	done
	$(GCOV_CMD) $(SOURCES)
	lcov -t "decimal" -o decimal.info -c -d . --ignore-errors unused --exclude '/usr/*'
	genhtml -o report decimal.info
	@echo "========================================"
	@echo "Coverage report successfully generated!"
	@echo "Open in browser: file://$(shell pwd)/report/index.html"
	@echo "========================================"
	@echo "Opening report in browser..."
	$(OPEN_CMD) report/index.html

clean:
	rm -f *.o *.a *.gcno *.gcda *.gcov *.info $(TEST_EXEC) $(TEST_EXEC_GCOV)
	rm -rf report
	rm -f valgrind_test.log valgrind_gcov.log
