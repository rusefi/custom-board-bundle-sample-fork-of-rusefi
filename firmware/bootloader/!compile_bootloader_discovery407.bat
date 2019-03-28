@echo off

echo Starting compilation for Discovery-407

rem set PROJECT_BOARD=Prometheus
rem set PROMETHEUS_BOARD=405
rem set EXTRA_PARAMS=-DDUMMY -DSTM32F405xx -DEFI_ENABLE_ASSERTS=FALSE -DCH_DBG_ENABLE_CHECKS=FALSE -DCH_DBG_ENABLE_TRACE=FALSE -DCH_DBG_ENABLE_ASSERTS=FALSE -DCH_DBG_ENABLE_STACK_CHECK=FALSE -DCH_DBG_FILL_THREADS=FALSE -DCH_DBG_THREADS_PROFILING=FALSE
set BOOTLOADER_CODE_PATH="."
rem set DEBUG_LEVEL_OPT="-O2"

call !compile_bootloader.bat -r

