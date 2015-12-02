#!/bin/bash

echo "Ejecutando 4 veces test.sh"
gnome-terminal -x ./test.sh &
gnome-terminal -x ./test.sh &
gnome-terminal -x ./test.sh &
gnome-terminal -x ./test.sh &

read -p "Pulsa Enter" VACIO

echo "Mostrando la lista"
cat /proc/smp-safe
