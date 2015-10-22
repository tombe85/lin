#!/bin/bash

echo cargando modulo
sudo insmod modlist.ko
echo "Insertando elementos 4 2 5 1 5 (echo add x > /proc/modlist)"
echo "add 4" > /proc/modlist
echo "add 2" > /proc/modlist
echo "add 5" > /proc/modlist
echo "add 1" > /proc/modlist
echo "add 5" > /proc/modlist
echo "Mostrando lista (cat /proc/modlist)"
cat /proc/modlist
echo "Ordenando lista (echo sort > /proc/modlist)"
echo "sort" > /proc/modlist
echo "Mostrando lista ordenada (cat /proc/modlist)"
cat /proc/modlist
echo "Eliminando elemento 5 (echo remove 5 > /proc/modlist)"
echo remove 5 > /proc/modlist
echo "Mostrando lista (cat /proc/modlist)"
cat /proc/modlist
echo "Limpiando la lista (echo cleanup > /proc/modlist)"
echo cleanup > /proc/modlist
echo "Mostrando la lista (cat /proc/modlist)"
cat /proc/modlist
echo "Eliminando modulo"
sudo rmmod modlist
echo "Mostrando el log del kernel"
dmesg | tail -n 14
echo Saliendo...
