#!/bin/bash                                                                                                                                                                       

SRV=$1
ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $SRV "cd /root/kvmperf/localtests; rm kernbench.txt; ./kernbench.sh"
