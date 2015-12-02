#!/bin/bash

echo "Insertando elementos 4 2 5 1 5 (echo add x > /proc/smp-safe)"
echo "add 4" > /proc/smp-safe
sleep 1
echo "add 2" > /proc/smp-safe
sleep 1
echo "add 5" > /proc/smp-safe
sleep 1
echo "add 1" > /proc/smp-safe
sleep 1
echo "add 5" > /proc/smp-safe
sleep 1
echo "Ordenando lista (echo sort > /proc/smp-safe)"
echo "sort" > /proc/smp-safe
