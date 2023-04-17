#!/bin/bash

# remove copy binary
sudo rm -f /usr/bin/ft_shield

# get pid of the backdoor
PID=$(ps -aux | grep ./ft_shield | grep root | awk '{print $2}') ; \
if [ -n "$PID" ]; then \
    sudo kill -15 $PID ; \
fi

# stop the service persistance
sudo service ft_shield stop 

# remove service file
sudo rm -f /etc/systemd/system/ft_shield.service

# reload systemctl daemon
sudo systemctl daemon-reload
