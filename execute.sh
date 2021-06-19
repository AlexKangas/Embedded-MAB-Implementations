#!/bin/sh

sed 's/|/\n/g' results/dataset/dataset.txt | sed 's/,/ /g' | sort -V | awk ' NR>1 {
k = $1 OFS $2
}
{
arr[k] += $3;
count[k]++
}
END{
for (i in arr){
print i, arr[i]/count[i]
}
}
' | sort -V | head -n-1 > results/dataset/res_dataset.txt
