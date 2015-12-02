#!/bin/bash

echo "Insertando elementos 4 2 5 1 5 (echo add x > /proc/modlist)"
echo "add 4" > /proc/modlist
sleep 1
echo "add 2" > /proc/modlist
sleep 1
echo "add 5" > /proc/modlist
sleep 1
echo "add 1" > /proc/modlist
sleep 1
echo "add 5" > /proc/modlist
sleep 1
echo "Ordenando lista (echo sort > /proc/modlist)"
echo "sort" > /proc/modlist
