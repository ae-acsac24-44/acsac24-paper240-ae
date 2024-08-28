#!/bin/bash                                                                                                                                                                       
SRV=$1

ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $SRV "service netperf start"
