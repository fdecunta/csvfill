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

TEST="tests"
EXPECTED="tests/expected"

test() {
	num="$1"
	short="$2"
	description="$3"
	return="$4"

	file1="${TEST}/${num}_${short}_db.csv"
	file2="${TEST}/${num}_${short}_input.csv"

	if [ ${return} -eq 0 ]; then
		expected="${EXPECTED}/${num}_${short}_expected.csv"
		./csvfill "$file1" "$file2" | diff -q "${expected}" -
	elif [ ${return} -eq 1 ]; then
		./csvfill "$file1" "$file2" 2>/dev/null
	fi

	if [ $? -ne ${return} ]; then
		echo "FAIL ${description}"
	else
		echo "OK   ${description}"
	fi
}

test "01" "good"       "Correct and vanilla"         0
test "02" "bad_column" "Throw error with bar column" 1
