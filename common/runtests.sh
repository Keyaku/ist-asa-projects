#!/bin/bash

################################################################################
# The MIT License
#
# Copyright (c) 2020 António Sarmento - https://github.com/Keyaku
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.
################################################################################

# =========== CONSTANTS ===========
readonly ScriptName="$0"
readonly DIR_script="$(cd "$(dirname "$0")" && pwd)"
readonly DIR_repo="$(git rev-parse --show-toplevel)"

# String Arrays
readonly usage_content=( "Usage: $(basename $ScriptName)"
"HELP:
	-h | --help : Shows this message
"
"FILES & DIRECTORIES:
	-d : Specify tests directory
	-e : Specify executable
	-f : Specify single test. Takes precedence over -d
"
"OPTIONS:
	-c | --compile [flags] : Runs Makefile for the project with given flags
	-m | --use-valgrind    : Enables testing with valgrind (good for memory tracking)
	-q | --quiet           : Quiet mode. Does not print to stdout (does not silence errors)
	-s | --single          : Stops on the first failure
	-t | --time            : Use system timer when running tests
"
)

# Loading external symbols
source "$DIR_repo/common/_base.env"
source "$DIR_repo/common/_test.env"

# Files & Directories
readonly DIR_current="$(pwd)"

# =========== VARIABLES ===========
FILE_tests=()

# Options
BOOL_compile=false
BOOL_quiet=false
BOOL_singleFail=false

# =========== FUNCTIONS ===========
function usage {
	for i in `seq 0 ${#usage_content[@]}`; do
		echo -e "${usage_content[i]}"
	done
    exit $RET_usage
}

function parse_args {
	if [ $# -eq 0 ]; then return 0; fi

	while [ $# -gt 0 ]; do
		case $1 in
			# FILES & DIRECTORIES
			-d )
				shift
				DIR_tests="$(get_absolute_dir "$1")"
				;;
			-e )
				shift
				EXEC_prog="$(get_absolute_dir $(dirname $1))/$(basename $1)"
				;;
			-f )
				shift
				FILE_tests=("$(get_absolute_dir $(dirname $1))/$(basename $1)")
				;;

			# OPTIONS
			-c | --make )
				BOOL_compile=true
				shift
				if [ $# -eq 0 ]; then continue; fi
				if [ ${1:0:1} = "-" ]; then continue; fi
				cflags="$1"
				;;

			-m | --use-valgrind )
				useValgrind="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
				;;

			-q | --quiet )
				BOOL_quiet=true
				;;

			-s | --stop )
				BOOL_singleFail=true
				;;

			-t | --time )
				useTimer="time"
				;;

			# HELP
			-h | --help )
				usage
				exit $RET_usage
				;;
			* ) printf "Unknown argument. \"$1\"\n"
				;;
		esac
		shift
	done

	return $RET_success
}

function set_defaults {
	DIR_project="$(cd "$DIR_script/.."; pwd)"
	if [ -z "$DIR_tests" ]; then
		DIR_tests="$DIR_script"
	fi

	EXEC_prog="$DIR_project/backbone"
}

function check_env {
	# Checking valgrind
	if [ "$useValgrind" -a -z "$(which valgrind)" ]; then
		print_error "valgrind is not installed."
		return $RET_error
	fi
}

function compile {
	# Checking executable
	if $BOOL_compile; then
		if [ ! -f "$DIR_project/Makefile" ]; then
			print_error "No Makefile present in $DIR_project."
			return $RET_error
		fi
		print_progress "Compiling project..."
		make -C "$DIR_project" ${cflags}
		return $?
	fi
}

# Target functionality
function test_list {
	local tested_count=0
	local fail_count=0
	local odd_count=0

	print_progress "\nBegin testing..."
	for test_input in "${FILE_tests[@]}"; do
		test_input="${test_input/$DIR_project\/}"
		local test_result="${test_input%.*}.diff"
		local test_outhyp="${test_input%.*}.${EXT_outhyp}"
		local test_output=""

		for ext in ${EXT_output[@]}; do
			test_output="${test_input%.*}.${ext}"
			test -f "$test_output" && break
		done

		# Skip incorrect files
		test $(StringContains "${test_input##*.}" "routes"; echo $?) -eq 0 && continue

		# Running program
		local test_name="$(basename "$test_input")"
		printf "Testing $test_name... "

		if [ "$useValgrind" ]; then
			$useValgrind "$EXEC_prog" "$test_input" "$EXT_outhyp"
		else
			$useTimer "$EXEC_prog" "$test_input" "$EXT_outhyp"
		fi

		if [ $? -ne 0 ]; then
			print_error "Failed execution."
			return $RET_error
		fi

		# Comparing results
		if [ -s "$test_output" ]; then
			diff "$test_outhyp" "$test_output" > "$test_result"

			# Act accordingly
			if [ -s "$test_result" ]; then
				print_failure "See file $test_result\n"
				fail_count=$(($fail_count + 1))
			else
				println "${Gre}√${RCol}"
				rm -f "$test_result" "$test_outhyp"
			fi
		else
			println "${Yel}?${RCol}"
			odd_count=$(($odd_count + 1))
		fi

		tested_count=$(($tested_count + 1))

		if [ $BOOL_singleFail == true ] && [ $fail_count -gt 0 ]; then
			break
		fi
	done

	local total_count=$(($fail_count + $odd_count))

	if [ $fail_count -gt 0 ]; then
		print_failure "Failed $fail_count / $tested_count tests (${#FILE_tests[@]} in total)."
	elif [ $odd_count -gt 0 ]; then
		println "Problems occurred in $odd_count / $tested_count tests (${#FILE_tests[@]} in total)."
	fi

	return $total_count
}

function get_test_list {
	# Gets and sorts the list of test files
	if [ ${#FILE_tests[@]} -eq 0 ]; then
		FILE_tests+=($(find $DIR_tests -type f -iname *.${EXT_input}*))
		FILE_tests=($(echo ${FILE_tests[@]} | xargs -n1 | sort | xargs))
	fi
}

function cleanup {
	: # Nothing to clean
}

function main {
	# Setting environment for testing
	parse_args "$@"
	set_defaults
	check_env
	if [ $? -eq $RET_error ]; then
		usage && exit $RET_error
	fi

	# Compiling project if necessary
	compile
	if [ $? -ne 0 ]; then
		exit $RET_error
	fi

	# Begin testing grounds
	local retval=$RET_success
	local fail_count=0
	get_test_list
	if [ $BOOL_quiet == true ]; then
		test_list > /dev/null
	else
		test_list
	fi
	fail_count=$?

	# Cleanup
	cleanup
	exit $fail_count
}

# Script starts HERE
main "$@"
