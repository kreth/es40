#!/bin/bash
success=0

function message() {
  echo -n -e '\033[1;36m' >&2
  echo $1 >&2
  echo -n -e '\033[0m' >&2
}

function message_red() {
  echo -n -e '\033[1;31m' >&2
  echo $1 >&2
  echo -n -e '\033[0m' >&2
}

function run_test() {
  local old_pwd=$(pwd)

  message "[test] Started test $1 at $(date)"

  cd "test/$1"
  ./test.sh
  local result=$?
  cd "$old_pwd"

  if [ "$result" -eq "0" ]
  then
    message "[test] Test $1 finished at $(date)" >&2
  else
    success=1
    message_red "[test] Test $1 failed at $(date)" >&2
  fi
}

message "[test] This is the ES40 test script"
message "[setup] preparing suite"
test -d test/tmp || mkdir -p test/tmp
if test ! -f test/tmp/cl67srmrom.exe; then
    if test -f test/artifacts/ES40FSL.ZIP; then
	message "[setup] cp file to test/tmp dir"
	unzip -d test/tmp test/artifacts/ES40FSL.ZIP
	bzcat test/artifacts/cl67srmrom.exe.bz2 >test/tmp/cl67srmrom.exe
    else
	message "missing ES40FSL.ZIP in artifacs dir!"
	exit 1
    fi
fi

run_test rom
run_test bootTru64_V4f
#run_test bootAlphaBios
#run_test installTru64

if [ "$success" -ne "0" ]
then
  message "[test] Some tests failed, please check the log"
else
  message "[test] All tests passed"
fi

exit "$success"
