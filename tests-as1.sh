#! /bin/bash

#
# Compile and boot with tests for assignment 1.
# The correct result of each test is signaled by
#
# Test passed...
#

fail() {
	echo
	echo "Failure: $1"
	exit 1
}

#
# The following tests are somewhat special, because they have to be evaluated
# interactivelly and they sometimes do not finish with the "Test passed..."
# banner.
#

for TEST in \
	tests/as1/panic1/test.c \
	tests/as1/exc1/test.c \
	tests/as1/exc2/test.c \
	tests/as1/exc3/test.c \
	tests/as1/io1/test.c \
	tests/as1/mutex5/test.c \
	; do
#	make distclean || fail "Cleanup before compilation"
	make -j3 "KERNEL_TEST=$TEST" || fail "Compilation"
	msim
#	make distclean "KERNEL_TEST=$TEST" || fail "Cleanup after compilation"
done

for TEST in \
	tests/as1/exc4/test.c \
	tests/as1/mutex1/test.c \
	tests/as1/mutex2/test.c \
	tests/as1/mutex3/test.c \
	tests/as1/mutex4/test.c \
	tests/as1/mutex6/test.c \
	tests/as1/thread1/test.c \
	tests/as1/thread2/test.c \
	tests/as1/timer1/test.c \
	; do
#	make distclean || fail "Cleanup before compilation"
	make -j3 "KERNEL_TEST=$TEST" || fail "Compilation"
	msim | tee /dev/stderr | grep '^Test passed\.\.\.$' > /dev/null || fail "Test $TEST"
#	make distclean "KERNEL_TEST=$TEST" || fail "Cleanup after compilation"
done

echo
echo "All tests passed..."
