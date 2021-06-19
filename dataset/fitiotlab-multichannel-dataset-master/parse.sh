#!/bin/sh

cat FIT-IoTLAB_dataset | sed 's/.*://' | sed 's/|/\n/g' | sed 's/,/ /g' | sort -V | awk ' NR>1 {
k = $1
}
{
arr[k] += $3;
count[k]++
}
END{
for (i in arr){
print arr[i]/count[i]
}
}
' | sed "1d" > datapdrs.txt
