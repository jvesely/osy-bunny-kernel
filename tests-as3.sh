#! /bin/bash

#
# Compile and boot with tests for assignment 3.
# The correct result of each test is signaled by
#
# Test passed...
#

fail() {
	rm -f test.log
	echo
	echo "Failure: $1"
	exit 1
}

# Don't output command executed by make unless run with -v
if [ "$1" == "-v" ] ; then
	SILENT_MAKE=""
else
	SILENT_MAKE="--silent"
fi

emake() {
	echo "Running make $SILENT_MAKE $@"
	make $SILENT_MAKE "$@"
}

#
# The following tests are somewhat special, because they have to be evaluated
# interactivelly and they sometimes do not finish with the "Test passed..."
# banner.
#

echo " *** Interactive tests ***"

for TEST in \
	tests/as3/io1/test.c \
	tests/as3/uspace1/test.c \
	tests/as3/uspace2/test.c \
	tests/as3/uspace3/test.c \
	; do
	emake distclean || fail "Cleanup before compilation"
	emake -j3 "USER_TEST=$TEST" || fail "Compilation"
	msim
	emake distclean "USER_TEST=$TEST" || fail "Cleanup after compilation"
done

echo " *** Non-interactive tests ***"

for TEST in \
	tests/as3/thread1/test.c \
	tests/as3/thread2/test.c \
	tests/as3/mutex1/test.c \
	tests/as3/mutex2/test.c \
	tests/as3/mutex3/test.c \
	tests/as3/mutex4/test.c \
	tests/as3/malloc1/test.c \
	; do
	emake distclean || fail "Cleanup before compilation"
	emake -j3 "USER_TEST=$TEST" || fail "Compilation"
	msim | tee test.log || fail "Execution"
	grep '^Test passed\.\.\.$' test.log > /dev/null || fail "Test $TEST"
	rm -f test.log
	emake distclean "USER_TEST=$TEST" || fail "Cleanup after compilation"
done

echo
echo "All tests passed..."
