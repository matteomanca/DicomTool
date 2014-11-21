#!/bin/bash
rm -f config.mk
touch config.mk
chmod 777 config.mk
#echo $(ls)
echo $(pwd)
echo $(uname -m)
echo $OSTYPE;
case "$OSTYPE" in
  solaris*) echo "SOLARIS" ;;
  darwin*)  
	echo "OSX" 
 	echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-mac\" > config.mk
    echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-mac\" >> config.mk
    echo WINLIB = >> config.mk
    #echo WINLIB = -lcharset -liconv >> config.mk
	;; 
  linux*)   
	echo "LINUX" 
	echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-ubuntu\" > config.mk
    echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-ubuntu\" >> config.mk
    echo WINLIB = >> config.mk
	;;
  cygwin*)   
	if [ $(uname -m) == "x86_64" ]
	then
		echo "cygwin 64 bit" 
		echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-cygwin\" > config.mk
		echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-cygwin\" >> config.mk
	else
		echo "cygwin 32 bit" 
		echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-cygwin32\" > config.mk
		echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-cygwin32\" >> config.mk	
	fi
#	echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-cygwin32\" > config.mk
#    echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-cygwin32\" >> config.mk
    echo WINLIB = -lcharset -liconv -lcharset -liconv  -ldcmrt -ldcmimgle -ldcmdata -loflog -lofstd -ltiff -ljpeg -lz -lpng -lz -liconv -lcharset -liconv -lcharset -lrt -lpthread  -lwsock32 -lnetapi32 >> config.mk
	;;	
  bsd*)     echo "BSD" ;;
  *)    echo INCLUDE_PATH = -I\"../lib/dcmtk-3.6.1/include-ubuntu\" > config.mk
        echo LIBB = -L\"../lib/dcmtk-3.6.1/lib-ubuntu\" >> config.mk
        echo WINLIB = >> config.mk
	 ;;
esac

#lib-cygwin

#make clean
#make all
