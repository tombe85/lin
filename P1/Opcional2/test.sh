#!/bin/bash

echo cargando modulo
sudo insmod modlist.ko
echo "Insertando elementos hola, que tal, bien, y tu, bien (echo add x > /proc/modlist)"
echo "add hola" > /proc/modlist
echo "add que tal" > /proc/modlist
echo "add bien" > /proc/modlist
echo "add y tu" > /proc/modlist
echo "add bien" > /proc/modlist
echo "Mostrando lista (cat /proc/modlist)"
cat /proc/modlist
echo "Eliminando elemento bien (echo remove bien > /proc/modlist)"
echo "remove bien" > /proc/modlist
echo "Mostrando lista (cat /proc/modlist)"
cat /proc/modlist
echo "Eliminando el elemento y tu (echo remove y tu > /proc/modlist)"
echo "remove y tu" > /proc/modlist
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
