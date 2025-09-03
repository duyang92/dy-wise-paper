#!/bin/bash
make 
echo "build done"
echo "running test"
sizelist=(20000 40000 60000 80000 100000)
rm result.txt
for j in ${sizelist[@]}
do
    ./text_actual_sim $j
    ./rec_actual_sim $j
    # touch result.txt
    # echo "running test $j sets"
    # ./test_icws_HTWE 4 $j 
    # ./test_fast_icws_HTWE 4 $j
    # ./test_bagminhash_HTWE 4 $j
    # ./test_dartminhash_HTWE 4 $j
    # ./test_dss2_SWE 16384 2 16384 2 4096 4 $j
    # ./test_dss2_SWE_LAZY 16384 2 16384 2 4096 4 $j
    # ./test_dartminhash_SWE 4 $j 16384 4 16384 4
    # python3 writecsv.py $j
    # rm result.txt
done


