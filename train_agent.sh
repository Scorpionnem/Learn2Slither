#!/bin/sh

echo Starting training sessions

./Learn2Slither --export models/1_11x11.txt --epochs 1 --train
./Learn2Slither --export models/10_11x11.txt --epochs 10 --train
./Learn2Slither --export models/100_11x11.txt --epochs 100 --train
./Learn2Slither --export models/1000_11x11.txt --epochs 1000 --train
./Learn2Slither --export models/10000_11x11.txt --epochs 10000 --train
./Learn2Slither --export models/100000_11x11.txt --epochs 100000 --train

echo Finished training sessions!
