#!/bin/bash
export LC_CTYPE=C
export LANG=C
export LC_ALL=C

# Download the firmware and put it into the test/tmp directory
#wget 'http://raymii.org/s/inc/downloads/es40-srmon/cl67srmrom.exe'

test -f ../tmp/decompressed.rom || { echo "rom test must first run succesfully!"; exit 1; }

# Start ES40_idb
if [[ -x ../../build/src/es40_idb ]]; then
  ../../build/src/es40_idb @test_file >es40_idb.log 2>&1 &
  ES40_PID=$!
fi

# Wait for ES40 emulator to start
sleep 5

# Connect to terminal
nc -t 127.0.0.1 21264 >es40.log &
NETCAT_PID=$!

# echo ES40_PID=$ES40_PID
# echo NETCAT_PID=$NETCAT_PID

timeout=60
while $(ps -q ${ES40_PID} >/dev/null); do
    sleep 1
    timeout=$(expr $timeout - 1)
    if [ $timeout -eq 0 ]; then
	echo "Timeout: Debugger does not finish after script execution!"
	kill ${ES40_PID}
	sleep 1
	$(ps -q ${ES40_PID} >/dev/null && kill -9 ${ES40_PID})
	exit 1
    fi
done
echo -n -e '\033[1;31m'
diff -c es40_correct.log es40.log
result=$?
[ $result -eq 0 ] && echo -e '\033[1;32mdiff clean\033[0m'
echo -n -e '\033[0m'
[ $result -eq 0 ] && rm -vf es40.log
test -f es40_idb.log && grep -q "RC r31" es40_idb.log || \
	( result=1 && echo -e '\033[1;31mmissing disassembly mnemonic in es40_idb.log\033[0m' )
[ $result -eq 0 ] && rm -vf es40_idb.log
exit $result
