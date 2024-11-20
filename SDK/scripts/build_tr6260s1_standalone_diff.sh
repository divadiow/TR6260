#!/bin/bash

if [ "$1" == "clean" ] ; then
make stc_tr6260s1_diffclean
fi

echo ""

make st_tr6260s1_diff

echo ""
echo "================================================"
echo "     generating tr6260s1.bin"
echo "================================================"

ls -l ../out/tr6260s1/standalone/xip.bin| awk '{printf "%08d", $5}' \
	> ../out/tr6260s1/standalone/tr6260s1_0x007000.bin
ls -l ../out/tr6260s1/standalone/standalone_tr6260s1.bin | awk '{printf "%08d", $5}' \
	>> ../out/tr6260s1/standalone/tr6260s1_0x007000.bin
cat ../out/tr6260s1/standalone/xip.bin ../out/tr6260s1/standalone/standalone_tr6260s1.bin \
	>> ../out/tr6260s1/standalone/tr6260s1_0x007000.bin
