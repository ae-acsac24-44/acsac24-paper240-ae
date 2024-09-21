#!/bin/bash                                                                                                                                                                       

SRV=$1
ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $SRV "cd /usr/src/kvmperf/localtests; rm hackbench.txt; ./hackbench.sh"
