#!/bin/sh
#sed -e "s/ 0.000000 /0.0000000 /g" -e "s/ 0.00000 /0.000000 /g" -e "s/ 0.0000 /0.00000 /g" -e "s/ 0.000 /0.0000 /g" -e "s/ 0.00 /0.000 /g" -e "s/ 0.000000$/0.0000000/g" -e "s/ 0.00000$/0.000000/g" -e "s/ 0.0000$/0.00000/g" -e "s/ 0.000$/0.0000/g" -e "s/ 0.00$/0.000/g" $1 > tyyy$1
sed -e "s/ 0.000000 /        0 /g" -e "s/ 0.00000 /       0 /g" -e "s/ 0.0000 /      0 /g" -e "s/ 0.000 /     0 /g" -e "s/ 0.00 /    0 /g" -e "s/ 0.000000$/        0/g" -e "s/ 0.00000$/       0/g" -e "s/ 0.0000$/      0/g" -e "s/ 0.000$/     0/g" -e "s/ 0.00$/    0/g" $1 > tyyy$1
mv tyyy$1 $1
