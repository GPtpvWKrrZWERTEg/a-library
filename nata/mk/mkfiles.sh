#!/bin/sh
# $Id: mkfiles.sh 46 2011-08-16 12:28:48Z mzbjyza2mji4zgz $

TARGET=./.files
rm -rf ${TARGET}
echo 'INPUT = \' >> ${TARGET}
find . -type f -name '*.c' -o -name '*.cpp' -o -name '*.h' | \
    awk '{ printf "%s \\\n", $1 }' | \
    grep -v 'check/' >> ${TARGET}
echo >> ${TARGET}
