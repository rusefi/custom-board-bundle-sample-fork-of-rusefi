
rd /s /q .dep

echo Starting compilation
rem the important piece
make -j12

call flash