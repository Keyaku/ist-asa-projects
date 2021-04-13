# How to use runtests.sh

In the project directory (e.g. `p1/`), create a directory `tests/` and link this script to that directory (one way would be `ln -s $(git rev-parse --show-toplevel)/common/runtests.sh`).
Then, simply create a directory with the testfiles, and the script should take it from there.

To run all tests, simply compile the project, then run `tests/runtests.sh`
and its default settings should do everything for you.

The options available for the script are:
* **HELP**
	* `-h | --help` : Shows "usage" message (kind of pointless, if you ask me)
* **FILES & DIRECTORIES**
	* `-d` : Specify tests directory (will run recursively only if children directories contain tests)
	* `-e` : Specify executable
	* `-f` : Specify single test. Takes precendence over `-d`

* **OPTIONS**
	* `-c | --compile [flags]` : Runs Makefile for the project with the given flags
	* `-m | --use-valgrind` : Enables testing with valgrind (good for memory tracking).
	Will abort if valgrind is not installed
	* `-q | --quiet` : Quiet mode. Does not print to stdout (will still print to stderr)
	* `-s | --single` : Stops on the first failure
	* `-t | --time` : Use system timer when running tests
