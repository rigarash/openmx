#!/bin/sh

set -ex

UPSTREAM_VERSION=$2
UPSTREAM_VERSION_RELEASE=`echo ${UPSTREAM_VERSION} | cut -d. -f 1,2`
ORIG_TARBALL=$3

WORKING_DIR=`dirname ${ORIG_TARBALL}`

REAL_TARBALL=`readlink -f ${ORIG_TARBALL}`
REAL_TARBALL_RELEASE="${WORKING_DIR}/openmx${UPSTREAM_VERSION_RELEASE}.tar.gz"

ORIG_TARBALL_DIR=`echo ${ORIG_TARBALL} | sed -e "s/_\(${UPSTREAM_VERSION}\)/-\1/g" -e "s/\.tar\.gz//g"`
ORIG_TARBALL_DIR_STRIP=`basename ${ORIG_TARBALL_DIR}`

[ -f ${REAL_TARBALL_RELEASE} ] || ( wget -P ${WORKING_DIR} http://www.openmx-square.org/openmx${UPSTREAM_VERSION_RELEASE}.tar.gz || exit 1 )

[ -d ${ORIG_TARBALL_DIR} ] && rm -rf ${ORIG_TARBALL_DIR}
mkdir -p ${ORIG_TARBALL_DIR}
# extract the source
tar --directory=${ORIG_TARBALL_DIR} --strip 1 -zxvf ${REAL_TARBALL_RELEASE} || exit 1
# apply the patch as requested in README.txt
tar --directory=${ORIG_TARBALL_DIR}/source -zxvf ${REAL_TARBALL} || exit 1
# remove the PDF without source - should we add +dfsg then?
rm -f ${ORIG_TARBALL_DIR}/*.pdf
rm -f ${ORIG_TARBALL} ${REAL_TARBALL} ${REAL_TARBALL_RELEASE}
# create the archive
GZIP=-9 tar --remove-files --directory ${WORKING_DIR} -czf ${ORIG_TARBALL} ${ORIG_TARBALL_DIR_STRIP} || exit 1

exit 0
