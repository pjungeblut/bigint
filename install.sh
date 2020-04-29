#!/bin/bash
# This script builds all dependencies of the bigint library.
# For Linux and Mac OS X systems.

CUR_DIR=`pwd`

# Installs the header files to the given location.
if [ $# -ge 1 ]; then
  BIGINT_INSTALL_PREFIX=${1}
else
  echo "ERROR: Specify a directory to install into."
  exit 1
fi

# Get absolute path name of installation directory.
mkdir -p "${BIGINT_INSTALL_PREFIX}/include/bigint" 2> /dev/null
cd "${BIGINT_INSTALL_PREFIX}" > /dev/null 2>&1
if [ $? != 0 ]; then
  echo "ERROR: Directory 'S{BIGINT_INSTALL_PREFIX}' does not exist nor could be created."
  echo "Please choose another directory."
  exit 1
else
  BIGINT_INSTALL_PREFIX=`pwd -P`
fi

echo "Library will be installed in '${BIGINT_INSTALL_PREFIX}'."

# Removing old files.
echo "Removing old files."
rm -rf "${BIGINT_INSTALL_PREFIX}/include/bigint"
if [ $? != 0 ]; then
  echo "WARNING: Could not remove old header files."
fi

# Install library.
cd ${CUR_DIR}
cp -r ./include "${BIGINT_INSTALL_PREFIX}/include/bigint"
if [ $? != 0 ]; then
  echo "ERROR: Installation failed."
  exit 1
fi

echo "SUCCESS: bigint was installed successfully."
echo "The bigint include files are located in '${BIGINT_INSTALL_PREFIX}/include'."
