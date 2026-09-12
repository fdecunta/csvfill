#!/bin/sh

#
# All tests are the same:
# - FILE1 	{number}_{short-description}_db.csv
# - FILE2  	{number}_{short-description}_input.csv
# - Expected 	{number}_{short-description}_expected.csv
#
# Long description should be a couple of words explaining 
# what is tested. 
# 

EXPECTED="tests/expected"

test() {
	num="$1"
	short="$2"
	description="$3"

	file1="tests/${num}_${short}_db.csv"
	file2="tests/${num}_${short}_input.csv"
	expected="tests/expected/${num}_${short}_expected.csv"

	./updatecsv "$file1" "$file2" | diff -q "${expected}" -

	if [ $? -ne 0 ]; then
		echo "FAIL ${description}"
	else
		echo "OK   ${description}"
	fi
}

test "1" "good" "Correct and vanilla"
