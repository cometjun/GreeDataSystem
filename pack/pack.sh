#/bin/bash

make clean -C DataServer/Debug/
make clean -C TransmitServer/Debug/
make clean -C HDFSFileSync/Debug/
make clean -C GuGongServer/Debug/
rm -rf DataServer/.git/
rm -rf TransmitServer/.git/
rm -rf HDFSFileSync/.git/
rm -rf GuGongServer/.git/
tar -zcf DataServer.tar.gz DataServer/ 
tar -zcf transmitserver.tar.gz TransmitServer/ 
tar -zcf HDFSFileSync.tar.gz HDFSFileSync/
tar -zcf GuGongServer.tar.gz GuGongServer/
tar -zcf autotools.tar.gz autotools/
tar -zcf test.tar.gz mgprs.py mclient.py messageserver.py

