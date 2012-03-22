#!/bin/sh

source $(dirname $0)/../../etc/checkEnvs.sh


UPD_CONFIG=dev
#ENABLE_VIDIMAX=1

BUILD_SCRIPT_FW=clinika_updateurl
#BUILD_WITHOUT_COMPONENTS_FW=1

#Comment that add to firmware pack name
SHORT_COMMENT=clinica_nsk_init

export UPD_CONFIG ENABLE_VIDIMAX BUILD_SCRIPT_FW BUILD_WITHOUT_COMPONENTS_FW SHORT_COMMENT

make -C $PRJROOT firmware

