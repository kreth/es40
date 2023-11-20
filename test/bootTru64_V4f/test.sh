#!/bin/bash
export LC_CTYPE=C
export LANG=C
export LC_ALL=C

# Download the firmware and put it in artefacts directory
# wget 'http://raymii.org/s/inc/downloads/es40-srmon/cl67srmrom.exe'

test -f ../tmp/decompressed.rom || { echo "rom test must first run succesfully!"; exit 1; }

rm -vf ../tmp/dka100.vdisk ../tmp/dpr.rom ../tmp/flash.rom

# Prepare disks and roms
test -f ../tmp/dka100.vdisk || tar -xj -C ../tmp  -f ../artifacts/dka100.vdisk.tar.bz
test -f ../tmp/dpr.rom || tar -xj -C ../tmp  -f ../artifacts/dpr.rom.tar.bz
test -f ../tmp/flash.rom || tar -xj -C ../tmp  -f ../artifacts/flash.rom.tar.bz

# Redirect to expect script
./test_simple.exp
result=$?
[ $result -eq 0 ] && echo -e '\033[1;32mtest successful\033[0m'
exit $result

#just for reference
#do_script=2
#while $(ps -q ${ES40_PID} >/dev/null); do
#    sleep 1
#    timeout=$(expr $timeout - 1)
#    if [ $timeout -eq 0 ]; then
#	echo "got timeout!"
#	break
#    fi
#    lastline=$(tail -n 1 es40_terminal.log)
#    echo line=$lastline
#    if [ "$lastline" == "P00>>>" ]; then
#	if [ $do_script -gt 0 ]; then
#	    do_script=$(expr $do_script - 1)
#	    echo -e "sh dev\r\n" >srv-input
#	else
#	    break
#	fi
#    fi
#done
#exec 3>&-
#sleep 1
#echo stopping es40 on pid ${ES40_PID}
#kill ${ES40_PID}
#sleep 1
#$(ps -q ${ES40_PID} >/dev/null && kill -9 ${ES40_PID})
#

#echo -n -e '\033[1;31m'
#diff -c es40_correct.log es40_terminal.log
#result=$?
#[ $result -eq 0 ] && echo -e '\033[1;32mdiff clean\033[0m'
#echo -n -e '\033[0m'
# leave logs only if test failed
#[ $result -eq 0 ] && rm -vf es40.log1
#[ $result -eq 0 ] && rm -vf es40_terminal.log1
#exit $result && [ timeout -eq 0 ]
