#!/bin/bash

echo cargando modulo
sudo insmod smp-safe.ko
echo "Insertando elementos 4 2 5 1 5 (echo add x > /proc/smp-safe)"
echo "add 4" > /proc/modlist
echo "add 2" > /proc/modlist
echo "add 5" > /proc/modlist
echo "add 1" > /proc/modlist
echo "add 5" > /proc/modlist
echo "Mostrando lista (cat /proc/smp-safe)"
cat /proc/smp-safe
echo "Ordenando lista (echo sort > /proc/smp-safe)"
echo "sort" > /proc/smp-safe
echo "Mostrando lista ordenada (cat /proc/smp-safe)"
cat /proc/smp-safe
echo "Eliminando elemento 5 (echo remove 5 > /proc/smp-safe)"
echo remove 5 > /proc/smp-safe
echo "Mostrando lista (cat /proc/smp-safe)"
cat /proc/smp-safe
echo "Limpiando la lista (echo cleanup > /proc/smp-safe)"
echo cleanup > /proc/smp-safe
echo "Mostrando la lista (cat /proc/smp-safe)"
cat /proc/smp-safe
echo "Eliminando modulo"
sudo rmmod smp-safe
echo "Mostrando el log del kernel"
dmesg | tail -n 14
echo Saliendo...
