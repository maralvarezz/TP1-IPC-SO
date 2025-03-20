#!/bin/bash

# Nombre del contenedor
CONTAINER_NAME="TPE-ARQUI"

# Iniciar el contenedor
docker start $CONTAINER_NAME

# Ejecutar los comandos make clean y make en las rutas especificadas
docker exec -it $CONTAINER_NAME make clean -C /root/Toolchain
docker exec -it $CONTAINER_NAME make clean -C /root
docker exec -it $CONTAINER_NAME make all -C /root/Toolchain
docker exec -it $CONTAINER_NAME make all -C /root

# echo "Proceso completado para el contenedor $CONTAINER_NAME"
docker stop $CONTAINER_NAME
#sudo chown $USER:$USER Image/x64BareBonesImage.qcow2
#chmod 664 Image/x64BareBonesImage.qcow2
./run.sh