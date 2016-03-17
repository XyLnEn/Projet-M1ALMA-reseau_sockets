#!/bin/bash

mate-terminal -e ./Serveur.exe &

for ((i = 0 ; 4 - $i ; i++))
do
   sleep 1
   mate-terminal -e ./Client.exe &
   
done
