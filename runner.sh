#!/bin/bash


echo "threads;check;shake\n" > res_times.csv
for(( i=1; i<=8; i++ ))
do 
	
	printf "$i;"
	./FATty check $i
	printf ";"
	./FATty shake $i;
	echo ""
done >> res_times.csv