#!/bin/bash

#Stop the service
echo "Stoping Service..."
systemctl stop ImageServer

#Disable Service Files 
echo "Disable systemctl service files"
systemctl disable ImageServer.service
systemctl disable ImageServerStopSO.service

#Remove Service Files
echo "Removing service files..."
rm /usr/lib/systemd/system/ImageServer.service
rm /usr/lib/systemd/system/ImageServerStopSO.service
rm /usr/bin/ImageServerStopSO.sh

#Remove bin file
echo "Removing bin file..."
rm /usr/bin/ImageServer
#Remove config file
echo "Removing config file..."
rm /etc/server/config.conf