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
	flags="$3"
	description="$4"
	return="$5"

	file1="${TEST}/${num}_${short}_db.csv"
	file2="${TEST}/${num}_${short}_input.csv"

	if [ ${return} -eq 0 ]; then
		expected="${EXPECTED}/${num}_${short}_expected.csv"
		./csvfill $flags "$file1" "$file2" | diff -q "${expected}" -
	elif [ ${return} -eq 1 ]; then
		./csvfill $flags "$file1" "$file2" 2>/dev/null
	fi

	if [ $? -ne ${return} ]; then
		echo "FAIL ${description}"
	else
		echo "OK   ${description}"
	fi
}

test "01" "good"       "" 	"Correct and vanilla"		0        
test "02" "bad_column" "" 	"Throw error with bar column" 	1
test "03" "quotes"     "" 	"Quoted fields" 		0
test "04" "overwrite"  "" 	"Attemp to overwrite value" 	1
test "05" "12opts"     "-1 2" 	"Use -1 flag for ID"	 	0
