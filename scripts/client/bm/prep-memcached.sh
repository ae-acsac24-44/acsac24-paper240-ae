#!/bin/bash                                                                                                                                                                       

SRV=$1

ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $SRV "sed -i 's/^-l\s\+127.0.0.1/-l 0.0.0.0/' /etc/memcached.conf; service memcached restart"
