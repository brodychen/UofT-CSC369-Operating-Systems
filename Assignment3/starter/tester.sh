#!/bin/bash
DIR=traceprogs
for file in tr-simpleloop.ref tr-matmul.ref tr-blocked.ref tr-test.ref; do
	echo "$file"
	for mem in 50 100 150 200; do
		echo "Memory $mem"
		for algo in rand fifo lru clock opt; do
			./sim -f $DIR/$file -m $mem -s 30000 -a $algo >> result.txt
		done
	done
done