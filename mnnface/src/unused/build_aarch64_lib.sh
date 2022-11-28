#!/bin/bash


set -x
set -e
make -j 4 CC_PREFIX="" TARGET_TOOLCHAIN_PREFIX="arm-himix200-linux-" 

