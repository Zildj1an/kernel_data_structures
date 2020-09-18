#!/bin/bash

sudo insmod datas.ko int_str="1,2,3,4,5"
cat /proc/datas
sudo rmmod datas
sudo dmesg | tail -n 7
