#!/bin/bash

mate-terminal -e ../Tp_reseau_sockets/Serveur.exe &

for ((i = 0 ; 4 - $i ; i++))
do
   mate-terminal -e ../Tp_reseau_sockets/Client.exe &
   sleep 1
done