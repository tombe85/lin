#!/bin/bash

echo "Ejecutando 4 veces test.sh"
./test.sh &
./test.sh &
./test.sh &
./test.sh &

read -p "Pulsa Enter" VACIO

echo "Mostrando la lista"
cat /proc/modlist
