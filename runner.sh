#!/bin/bash

for(( i=1; i<=8; i++ ))
do 
	echo "threads;check;shake\n" > res_times.csv
	echo "$i;" >> res_times.csv
	{ ./FATty check $i>> res_times.csv ; }
	echo ";" >> res_times.csv
	{ ./FATty shake $i> res_times.csv ; }
done