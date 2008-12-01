#! /bin/bash

#
# Compile and boot with tests for assignment 2.
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

test() {
	emake distclean || fail "Cleanup before compilation"
	emake "KERNEL_TEST=$1" || fail "Compilation"
	msim | tee test.log || fail "Execution"
	grep '^Test passed\.\.\.$' test.log > /dev/null || fail "Test $1"
	rm -f test.log
	emake distclean "KERNEL_TEST=$1" || fail "Cleanup after compilation"
}

for TEST in \
	tests/as2/area1/test.c \
	tests/as2/falloc1/test.c \
	tests/as2/malloc1/test.c \
	tests/as2/thrcopy1/test.c \
	; do
	test "${TEST}"
done

# these tests need a bit of special handling to compile and link

for TEST in \
	tests/as2/map1 \
	tests/as2/map2 \
	; do
	cp -f kernel/$TEST/test.h kernel/tests/as2/shared/
	test "${TEST}/test.c tests/as2/shared/tst_area_list.c tests/as2/shared/tst_area_ops.c"
	rm kernel/test/as2/shared/test.h
done

echo
echo "All tests passed..."
