#!/bin/bash

# check the backdoor is listening on port 4242
sudo netstat -tulnp | grep 4242

# check the backdoor process is running
sudo ps -aux | grep ./ft_shield | grep root | grep -v grep

# check binary copy has been created successfully
ls -l /usr/bin/ft_shield

# check difference between original binary and the copy
diff /usr/bin/ft_shield ft_shield

# check that service file has been created correctly
ls -l /etc/systemd/system/ft_shield.service

# verify service status
sudo service ft_shield status
