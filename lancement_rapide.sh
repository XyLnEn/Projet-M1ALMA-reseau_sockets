#!/bin/bash

mate-terminal -e ./Serveur.exe &

for ((i = 0 ; 4 - $i ; i++))
do
   mate-terminal -e ./Client.exe &
   sleep 1
done
