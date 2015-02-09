#!/bin/bash
# This job runs a nightly build, remember to set jenkins to:
# 1) set the workspace to point to repos/ACS/trunk on the svn server
# 2) Poll svn for changes at night
# 3) trigger mantis changes and file a ticket if project does not build

source ~/.bashrc
export TARGETSYS=MED
export INTROOT=/system/introot/EXECUTOR_$EXECUTOR_NUMBER
export ACS_CDB=$WORKSPACE/Medicina
rm -rf $INTROOT
mkdir -p $INTROOT
getTemplateForDirectory INTROOT $INTROOT
cd SystemMake
make common-clean common-build
