#!/bin/bash

echo cargando modulo
sudo insmod modlist.ko
echo "Insertando elementos 4 2 5 1 5"
echo "add 4" > /proc/modlist
echo "add 2" > /proc/modlist
echo "add 5" > /proc/modlist
echo "add 1" > /proc/modlist
echo "add 5" > /proc/modlist
echo "Mostrando lista"
cat /proc/modlist
echo "Ordenando lista"
echo "sort" > /proc/modlist
echo "Mostrando lista ordenada"
cat /proc/modlist
echo eliminando elemento 5
echo remove 5 > /proc/modlist
echo Mostrando lista
cat /proc/modlist
echo Eliminando modulo
sudo rmmod modlist
