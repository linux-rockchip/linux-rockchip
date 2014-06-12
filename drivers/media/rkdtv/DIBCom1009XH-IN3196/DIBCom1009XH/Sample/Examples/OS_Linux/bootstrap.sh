#! /bin/sh
BASE=`pwd`
BRIDGE=$BASE/../../../Bridge
DRIVER=$BASE/../../../Driver
BRIDGE_TARGET=$BRIDGE/${1}
DRIVER_TARGET=$DRIVER/${2}

#ARCH=LinuxKernelDib07x0

if [ ! -d ${BRIDGE} ]
then
  echo "Error, directory ${BRIDGE} does not exist"
  exit 1
fi

if [ ! -d ${DRIVER} ]
then
  echo "Error, directory ${DRIVER} does not exist"
  exit 1
fi

if [ ! -d ${BRIDGE_TARGET} ]
then
  echo "Error, directory ${BRIDGE_TARGET} does not exist"
  exit 1
fi

if [ ! -d ${DRIVER_TARGET} ]
then
  echo "Error, directory ${DRIVER_TARGET} does not exist"
  exit 1
fi

if [ -e "${BRIDGE}/Target" ]
then
    if [ -h "${BRIDGE}/Target" ]
    then
      echo "Removing ${BRIDGE}/Target"
      rm -f "${BRIDGE}/Target" 
    else
      echo "${BRIDGE}/Target is not a symbolic link !!"
      exit 1
    fi
fi

if [ -e "${DRIVER}/Target" ]
then
    if [ -h "${DRIVER}/Target" ]
    then
      echo "Removing ${DRIVER}/Target"
      rm -f "${DRIVER}/Target" 
    else
      echo "${DRIVER}/Target is not a symbolic link !!"
      exit 1
    fi
fi

if [ -e "./VB" ]
then
    if [ -h "./VB" ]
    then
      echo "Removing ./VB"
      rm -f "./VB" 
    else
      echo "./VB is not a symbolic link !!"
      exit 1
    fi
fi

if [ -e "./VBL" ]
then
    if [ -h "./VBL" ]
    then
      echo "Removing ./VBL"
      rm -f "./VBL" 
    else
      echo "./VBL is not a symbolic link !!"
      exit 1
    fi
fi

if [ -e "./DAB_VB" ]
then
    if [ -h "./DAB_VB" ]
    then
      echo "Removing ./DAB_VB"
      rm -f "./DAB_VB" 
    else
      echo "./DAB_VB is not a symbolic link !!"
      exit 1
    fi
fi

cd ${BRIDGE}
echo "Creating bridge link :  Target -> ${1}"
ln -s $1 Target
cd ${DRIVER}
echo "Creating driver link :  Target -> ${2}"
ln -s $2 Target
cd $BASE
if [ -e "${3}/VB" ]
then
ln -s ${3}/VB VB
fi
if [ -e "${3}/VBL" ]
then
ln -s ${3}/VBL VBL
fi
if [ -e "${3}/DAB_VB" ]
then
ln -s ${3}/DAB_VB DAB_VB
fi
