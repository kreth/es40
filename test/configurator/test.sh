#!/bin/bash
export LC_CTYPE=C
export LANG=C
export LC_ALL=C

# Start ES40
  ../../build/src/es40 >es40.log 2>&1 &
  ES40_PID=$!

# Wait for ES40 emulator to fail, because the config has a missing semicolon
sleep 1

ps -q ${ES40_PID} && { echo "expected es40 to die, test failed"; kill ${ES40_PID}; exit 1; } 

diff -c es40_correct.log es40.log
result=$?
[ $result -eq 0 ] && echo -e '\033[1;32mdiff clean\033[0m'
echo -n -e '\033[0m'
[ $result -eq 0 ] && rm -f es40.log
exit $result
