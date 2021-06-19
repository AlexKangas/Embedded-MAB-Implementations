#!/bin/sh

make main
for i in {1..40}
do
    ./main 50 >> results/50.txt &
    ./main 100 >> results/100.txt &
    ./main 200 >> results/200.txt &
    ./main 300 >> results/300.txt &
    ./main 400 >> results/400.txt &
    ./main 500 >> results/500.txt &
    ./main 600 >> results/600.txt &
    ./main 700 >> results/700.txt &
    ./main 800 >> results/800.txt &
    ./main 900 >> results/900.txt &
    ./main 1000 >> results/1000.txt &
    
done
