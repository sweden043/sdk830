#!/bin/sh

source $(dirname $0)/../../etc/checkEnvs.sh
source $PRJROOT/src/update/default_env.sh

UPD_CONFIG=rel
#ENABLE_VIDIMAX=1

make -C $PRJROOT firmware
