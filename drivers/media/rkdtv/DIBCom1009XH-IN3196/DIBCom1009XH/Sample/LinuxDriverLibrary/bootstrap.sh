#! /bin/sh
BASE=`pwd`
BRIDGE=$BASE/../../Bridge
DRIVER=$BASE/../../Driver
BRIDGE_TARGET=$BRIDGE/${1}
DRIVER_TARGET=$DRIVER/${2}

#ARCH=linux_kernel_pc

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
      rm -f "${DRIVER}/Target" 
    else
      echo "${DRIVER}/Target is not a symbolic link !!"
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


