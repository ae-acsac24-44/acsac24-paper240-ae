#!/bin/bash                                                                                                                                                                       

SRV=$1

echo "Grab results for $SRV" 

ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $SRV "cd /usr/src/kvmperf/localtests; cat kernbench.txt; rm kernbench.txt" >> kernbench.txt
