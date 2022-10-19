#!/bin/bash

# STM32F4...

SCRIPT_NAME="compile_nucleo_f429.sh"
echo "Entering $SCRIPT_NAME"

# Nucleo boards use MCO signal from St-Link and NOT oscillator - these need STM32_HSE_BYPASS

export USE_FATFS=no

export EXTRA_PARAMS="-DDUMMY \
 -DEFI_INJECTOR_PIN3=Gpio::Unassigned \
 -DSTM32_HSE_BYPASS=TRUE \
 \
 -DEFI_COMMUNICATION_PIN=Gpio::B7 \
 -DLED_CRITICAL_ERROR_BRAIN_PIN=Gpio::B14 \
 -DEFI_ENABLE_ASSERTS=FALSE \
 -DCH_DBG_ENABLE_CHECKS=FALSE -DCH_DBG_ENABLE_ASSERTS=FALSE -DCH_DBG_ENABLE_STACK_CHECK=FALSE -DCH_DBG_FILL_THREADS=FALSE -DCH_DBG_THREADS_PROFILING=FALSE"
export DEBUG_LEVEL_OPT="-O2"

bash ../common_make.sh nucleo_f429 ARCH_STM32F4