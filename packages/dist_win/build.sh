#!/bin/sh
#
# Requirements:
#
# o Visual Studio 2005
# o Intel(R) Fortran Compiler 9.1
# o Visual Studio 6 w/ SP5
# o Visual Fortran 6.1a
# o InstallShield Professional 6.31
# o ModelViewer 1.0
# o jdk >= 1.4.1
# o hdf5 >= 1.6.5
# o htmlhelp
# o cygwin
# o   sh/bash
# o   tar
# o   bzip
# o   gzip
# o   tar
# o   patch
# o   ant
# o   locate
# o   diff
# o   unix2dos
#
#
# build using:
# ./phast-1.0-1.sh cvsexport
# time ./phast-1.0-1.sh all 2>&1 | tee phast-1.0-1.sh.build.log
#
##################################################################

# echo everything
set -x

# find out where the build script is located
tdir=`echo "$0" | sed 's%[\\/][^\\/][^\\/]*$%%'`
test "x$tdir" = "x$0" && tdir=.
scriptdir=`cd $tdir; pwd`
# find src directory.  
# If scriptdir ends in SPECS, then topdir is $scriptdir/.. 
# If scriptdir ends in CYGWIN-PATCHES, then topdir is $scriptdir/../..
# Otherwise, we assume that topdir = scriptdir
topdir1=`echo ${scriptdir} | sed 's%/SPECS$%%'`
topdir2=`echo ${scriptdir} | sed 's%/CYGWIN-PATCHES$%%'`
if [ "x$topdir1" != "x$scriptdir" ] ; then # SPECS
  topdir=`cd ${scriptdir}/..; pwd`
else
  if [ "x$topdir2" != "x$scriptdir" ] ; then # CYGWIN-PATCHES
    topdir=`cd ${scriptdir}/../..; pwd`
  else
    topdir=`cd ${scriptdir}; pwd`
  fi
fi

tscriptname=`basename $0 .sh`
export PKG=`echo $tscriptname | sed -e 's/\-[^\-]*\-[^\-]*$//'`
export VER=`echo $tscriptname | sed -e "s/${PKG}\-//" -e 's/\-[^\-]*$//'`
export REL=`echo $tscriptname | sed -e "s/${PKG}\-${VER}\-//"`
export MAJOR=`echo $VER | sed -e 's/\.[^.]*//g'`
export MINOR=`echo $VER | sed -e 's/[^\.]*\.//' -e 's/\.[^\.]*//'`
export BASEPKG=${PKG}-${VER}-${REL}
export FULLPKG=${BASEPKG}
LOWER='abcdefghijklmnopqrstuvwxyz'
UPPER='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
export VER_UC=`echo $VER | sed -e "y/$LOWER/$UPPER/"`
export DIFF_IGNORE="-x *.aps -x *.ncb -x *.opt"

export src_orig_pkg_name=${FULLPKG}.tar.gz
export src_pkg_name=${FULLPKG}-src.tar.bz2
export src_patch_name=${FULLPKG}.patch
export bin_pkg_name=${FULLPKG}.tar.bz2
export src_orig_pkg=${topdir}/${src_orig_pkg_name}
export src_orig_pkg_mv=${topdir}/Mv-${VER}.tar.gz
export src_pkg=${topdir}/${src_pkg_name}
export src_patch=${topdir}/${src_patch_name}
export bin_pkg=${topdir}/${bin_pkg_name}
export srcdir=${topdir}/${BASEPKG}
export objdir=${srcdir}/.build
export instdir=${srcdir}/.inst
export srcinstdir=${srcdir}/.sinst
export checkfile=${topdir}/${FULLPKG}.check
export cvstag=version-`echo "${VER}" | sed -e 's/\./_/g'`
# run on
host=i686-pc-cygwin
# if this package creates binaries, they run on
target=i686-pc-cygwin
prefix=/${PKG}-${VER}
sysconfdir=/etc
MY_CFLAGS="-O2 -g"
MY_LDFLAGS=

# use Visual Studio 2005 to compile
DEVENV="devenv.exe"
PHAST_SLN=`cygpath -w ./src/phast/win32_2005/phastpp.sln`
PHASTINPUT_SLN=`cygpath -w ./src/phastinput/vc80/phastinput.sln`
PHASTHDF_SLN=`cygpath -w ./src/phasthdf/win32/phastexport.sln`
MSI_SLN=`cygpath -w ./msi/msi.sln`


# InstallShield settings (based on exported build file
# IS_COMPILER=`locate Compile.exe | grep InstallShield`
# IS_BUILDER="`locate ISBuild.exe | grep InstallShield`"
IS_COMPILER="/cygdrive/c/Program Files/Common Files/InstallShield/IScript/Compile.exe"
IS_BUILDER="/cygdrive/c/Program Files/InstallShield/Professional - Standard Edition/Program/ISBuild.exe"

IS_INSTALLPROJECT=`cygpath -w "${objdir}/packages/win32-is/phast.ipr"`
IS_CURRENTBUILD=SingleDisk

IS_HOME=`echo "${IS_BUILDER}" | sed -e 's^/Program/ISBuild.exe$^^'`
IS_HOME=`cygpath -w "${IS_HOME}"`

IS_INCLUDEIFX=${IS_HOME}\\Script\\IFX\\Include
IS_INCLUDEISRT=${IS_HOME}\\Script\\ISRT\\Include
IS_INCLUDESCRIPT=`cygpath -w "${objdir}/packages/win32-is/Script Files"`
IS_LINKPATH1="-LibPath${IS_HOME}\\Script\\IFX\\Lib"
IS_LINKPATH2="-LibPath${IS_HOME}\\Script\\ISRT\\Lib"
IS_RULFILES=`cygpath -w "${objdir}/packages/win32-is/Script Files/Setup.rul"`
IS_LIBRARIES="isrt.obl ifx.obl"
IS_DEFINITIONS=""
IS_SWITCHES="-w50 -e50 -v3 -g"
export PHASTTOPDIR="`cygpath -w "${instdir}${prefix}"`"

# Modelviewer 
###export MODELVIEWER="/cygdrive/c/Program Files/USGS/Model Viewer 1.0/"
export MODELVIEWER_1_3="/cygdrive/c/Program Files/USGS/Model Viewer 1.3/"

# Examples
EXAMPLES="
 diffusion1d \
 diffusion2d \
 disp2d \
 ex1 \
 ex2 \
 ex3 \
 ex4 \
 ex4_ddl \
 ex4_noedl \
 ex4restart \
 ex4_start_time \
 ex4_transient \
 kindred4.4 \
 leaky \
 leakysurface \
 leakyx \
 leakyz \
 linear_bc \
 linear_ic \
 phrqex11 \
 property \
 radial \
 river \
 shell \
 simple \
 unconf \
 well \
 zf"

mkdirs() {
  (cd ${topdir} && \
  mkdir -p ${objdir} && \
  mkdir -p ${instdir} && \
  mkdir -p ${srcinstdir} )
}

precheck() {
  (
  if [ ! -s "`which ${DEVENV}`" ] ; then \
    echo "Error: Can't find Microsoft Visual Studio 8 (2005): ${DEVENV}"; \
    exit 1; \
  fi && \
###  if [ "x$DEV_VTK_LIBDLL" = "x" ] ; then \
###    echo "Error: DEV_VTK_LIBDLL must be set"; \
###    exit 1; \
###  fi && \
  if [ "x$DEV_HDF5_LIBDLL" = "x" ] ; then \
    echo "Error: DEV_HDF5_LIBDLL must be set"; \
    exit 1; \
  fi && \
  if [ "x$DEV_HDF5_INC" = "x" ] ; then \
    echo "Error: DEV_HDF5_INC must be set"; \
    exit 1; \
  fi && \
###  if [ "x$DEV_MPICH_INC" = "x" ] ; then \
###    echo "Error: DEV_MPICH_INC must be set"; \
###    exit 1; \
###  fi && \
###  if [ "x$DEV_MPICH_LIB" = "x" ] ; then \
###    echo "Error: DEV_MPICH_LIB must be set"; \
###    exit 1; \
###  fi && \
###  if [ "x$DEV_VTK_INC" = "x" ] ; then \
###    echo "Error: DEV_VTK_INC must be set"; \
###    exit 1; \
###  fi && \
  if [ "x$DEV_ZLIB122_INC" = "x" ] ; then \
    echo "Error: DEV_ZLIB122_LIB must be set"; \
    exit 1; \
  fi && \
  if [ "x$DEV_ZLIB122_LIB" = "x" ] ; then \
    echo "Error: DEV_ZLIB122_LIB must be set"; \
    exit 1; \
  fi && \
  if [ ! -d "${MODELVIEWER_1_3}" ] ; then \
    echo "Error: ${MODELVIEWER_1_3} not found"; \
    echo "Error: ModelViewer must be installed"; \
    exit 1; \
  fi )
}

#### Note: cp -al and ln aren't working if no ownership of source
#### after upgrading xp/cygwin 12/15/2004
####
###cvsexport() {
###  (precheck && \
###  cd ${topdir} && \
###  cvs export -r ${cvstag} -d Mv mv_phast/Mv && \
###  cd ${topdir} && \
#### external files
###  mkdir -p Redist && \
###  cp "`cygpath ${DEV_VTK_LIBDLL}`/vtkdll.dll"           ${topdir}/Redist/. && \
###  cp "`cygpath ${DEV_HDF5_LIBDLL}`/hdf5dll.dll"         ${topdir}/Redist/. && \
###  cp "`cygpath ${DEV_HDF5_LIBDLL}`/../bin/zlib1.dll"    ${topdir}/Redist/. && \
###  cp "`cygpath ${DEV_HDF5_LIBDLL}`/../bin/szlibdll.dll" ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/notice.txt"            ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/readme.txt"            ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/doc/ofr02-106.pdf"     ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/bin/modview.chm"       ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/bin/DFORRT.DLL"        ${topdir}/Redist/. && \
###  cp "`cygpath "${MODELVIEWER}"`/bin/lf90.eer"          ${topdir}/Redist/. && \
###  tar cvzf ${src_orig_pkg_mv} Mv Redist && \
###  rm -rf Mv Redist )
###}

# Note: cp -al and ln aren't working if no ownership of source
# after upgrading xp/cygwin 12/15/2004
#
svnexport() {
  (precheck && \
  cd ${topdir} && \
  svn export -r ${REL} http://internalbrr.cr.usgs.gov/svn_GW/ModelViewer/trunk Mv && \
  cd ${topdir} && \
# external files
  mkdir -p Redist && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/notice.txt"            ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/readme.txt"            ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/doc/ofr02-106.pdf"     ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/DFORRT.DLL"        ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/hdf5dll.dll"       ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/lf90.eer"          ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/lf90wiod.dll"      ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/modview.chm"       ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/szlibdll.dll"      ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/vtkCommon.dll"     ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/vtkFiltering.dll"  ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/vtkGraphics.dll"   ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/vtkImaging.dll"    ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/vtkRendering.dll"  ${topdir}/Redist/. && \
  cp "`cygpath "${MODELVIEWER_1_3}"`/bin/zlib1.dll"         ${topdir}/Redist/. && \
  tar cvzf ${src_orig_pkg_mv} Mv Redist && \
  rm -rf Mv Redist )
}

prep() {
  (cd ${topdir} && \
  tar xvzf ${src_orig_pkg} && \
  cd ${topdir}/${FULLPKG} && \
##  tar xvzf ${src_orig_pkg_mv} && \
  cd ${topdir} && \
  if [ -f ${src_patch} ] ; then \
    patch -p0 --binary < ${src_patch} ;\
  fi && \
  mkdirs )
}

conf() {
  (cd ${objdir} && \
  rm -rf * && \
# copy links to ${objdir} for building
  find ${srcdir} -mindepth 1 -maxdepth 1 ! -name .build ! -name .inst ! -name .sinst -exec cp -al {} . \; && \
# create missing InstallShield directories if nec
  if [ ! -d "${objdir}/packages/win32-is/Setup Files/Compressed Files/Language Independent/Intel 32" ] ; then \
    mkdir -p "${objdir}/packages/win32-is/Setup Files/Compressed Files/Language Independent/Intel 32" ; \
  fi && \
  if [ ! -d "${objdir}/packages/win32-is/Setup Files/Compressed Files/0009-English/OS Independent" ] ; then \
    mkdir -p "${objdir}/packages/win32-is/Setup Files/Compressed Files/0009-English/OS Independent" ; \
  fi && \
  if [ ! -d "${objdir}/packages/win32-is/Setup Files/Compressed Files/0009-English/Intel 32" ] ; then \
    mkdir -p "${objdir}/packages/win32-is/Setup Files/Compressed Files/0009-English/Intel 32" ; \
  fi && \
# make sure phast.ipr exists and has a size greater than zero
  if [ ! -s ${srcdir}/packages/win32-is/phast.ipr ] ; then \
    echo "Error: MISSING: ${srcdir}/packages/win32-is/phast.ipr"; \
    exit 1; \
  fi && \
# make sure value.shl exists and has a size greater than zero
  if [ ! -s ${srcdir}/packages/win32-is/String\ Tables/0009-English/value.shl ] ; then \
    echo "Error: MISSING: ${srcdir}/packages/win32-is/String\ Tables/0009-English/value.shl"; \
    exit 1; \
  fi )
}

build() {
  (cd ${objdir} && \
# build phasthdf.exe
  "${DEVENV}" "${PHASTHDF_SLN}" /out phasthdf.log /build Release && \
# build phastinput.exe
  "${DEVENV}" "${PHASTINPUT_SLN}" /out phastinput.log /build Release && \
# build phast.jar
  ant -buildfile ./src/phasthdf/build.xml dist-Win32 && \
# build merge/phast.exe
  "${DEVENV}" "${PHAST_SLN}" /out phast-merge.log /build merge && \
# build ser/phast.exe
  "${DEVENV}" "${PHAST_SLN}" /out phast-ser.log /build ser && \
# build phast.msi
#  "${DEVENV}" "${MSI_SLN}" /out msi.log /build Release )
  MSBuild.exe "${MSI_SLN}" /p:Configuration=Release /p:TargetName=${FULLPKG} /p:Major=${MAJOR} /p:Minor=${MINOR} /p:Build=${REL} )
}


check() {
  (cd ${objdir} && \
# TODO (ie make test | tee ${checkfile} 2>&1)
  echo Check complete. )
}

clean() {
  (cd ${objdir} && \
###  msdev `cygpath -w ./phastexport/phastexport.dsw` /MAKE "phastexport - Win32 Release" /CLEAN && \
###  msdev `cygpath -w ./srcinput/win32/phastinput.dsw` /MAKE "phastinput - Win32 Release" /CLEAN && \
  ant -buildfile export/build.xml clean )
# clean phast
###  msdev `cygpath -w ./srcphast/win32/phast.dsw` /MAKE "phast - Win32 merge" /CLEAN && \
###  msdev `cygpath -w ./srcphast/win32/phast.dsw` /MAKE "phast - Win32 ser" /CLEAN && \
# clean ModelViewer
###  msdev `cygpath -w ./Mv/MvProject.dsw` /MAKE "ModelViewer - Win32 Release" /CLEAN )
}

install() {
  (cd ${instdir} && \
# create directory structure
  /usr/bin/mkdir -p ${instdir}/logs && \
  /usr/bin/mkdir -p ${instdir}${prefix}/bin && \
  /usr/bin/mkdir -p ${instdir}${prefix}/doc && \
  /usr/bin/mkdir -p ${instdir}${prefix}/database && \
  /usr/bin/mkdir -p ${instdir}${prefix}/examples && \
# create examples directories
  for arg in $EXAMPLES; do
    /usr/bin/mkdir -p ${instdir}${prefix}/examples/${arg}
  done && \
  /usr/bin/mkdir -p ${instdir}${prefix}/mv && \
  /usr/bin/mkdir -p ${instdir}${prefix}/mv/doc && \
  /usr/bin/mkdir -p ${instdir}${prefix}/mv/bin && \
  /usr/bin/mkdir -p ${instdir}${prefix}/export/bin && \
  /usr/bin/mkdir -p ${instdir}${prefix}/export/lib && \
  /usr/bin/mkdir -p ${instdir}${prefix}/export/lib/Win32 && \
  /usr/bin/install -m 644 ${objdir}/doc/README \
    ${instdir}${prefix}/README.txt && \
# (bin dir) phast-ser, phast-mpich2, hdf5dll.dll, zlib1.dll, szlibdll.dll phast.bat
  /usr/bin/install -m 755 ${objdir}/src/phastinput/vc80/Release/phastinput.exe \
    ${instdir}${prefix}/bin/phastinput.exe && \
  /usr/bin/install -m 755 ${objdir}/src/phast/win32_2005/merge/phast.exe \
    ${instdir}${prefix}/bin/phast-mpich2.exe && \
  /usr/bin/install -m 755 ${objdir}/src/phast/win32_2005/ser/phast.exe \
    ${instdir}${prefix}/bin/phast-ser.exe && \
  /usr/bin/install -m 755 ${objdir}/Redist/hdf5dll.dll \
    ${instdir}${prefix}/bin/hdf5dll.dll && \
  /usr/bin/install -m 755 ${objdir}/Redist/zlib1.dll \
    ${instdir}${prefix}/bin/zlib1.dll && \
  /usr/bin/install -m 755 ${objdir}/Redist/szlibdll.dll \
    ${instdir}${prefix}/bin/szlibdll.dll && \
  /usr/bin/install -m 755 ${objdir}/bin/${PKG}.bat \
    ${instdir}${prefix}/bin/${PKG}.bat && \
# doc directory
  /usr/bin/install -m 644 ${objdir}/doc/phast.pdf \
    ${instdir}${prefix}/doc/phast.pdf && \
  /usr/bin/install -m 644 ${objdir}/doc/phreeqc.pdf \
    ${instdir}${prefix}/doc/phreeqc.pdf && \
  /usr/bin/install -m 644 ${objdir}/doc/wrir02-4172.pdf \
    ${instdir}${prefix}/doc/wrir02-4172.pdf && \
  /usr/bin/install -m 644 ${srcdir}/src/phast/phreeqc.revisions \
    ${instdir}${prefix}/doc/phreeqc.revisions.txt && \
  /usr/bin/unix2dos ${instdir}${prefix}/doc/phreeqc.revisions.txt && \
  /usr/bin/install -m 644 ${srcdir}/src/phast/revisions \
    ${instdir}${prefix}/doc/RELEASE.txt && \
  /usr/bin/unix2dos ${instdir}${prefix}/doc/RELEASE.txt && \
# src directories
  /usr/bin/cp -r ${srcdir}/src ${instdir}${prefix}/. && \
  /usr/bin/rm -rf ${instdir}${prefix}/src/phasthdf/hdf-java &&\
  /usr/bin/rm -rf ${instdir}${prefix}/src/phasthdf/test &&\
  /usr/bin/find ${instdir}${prefix}/src/phasthdf/. -type f -name ".nbattrs" -exec /usr/bin/rm -f {} \; && \
# src/phast directory
  /usr/bin/rm -f ${instdir}${prefix}/src/phast/phreeqc.revisions &&\
  /usr/bin/rm -rf ${instdir}${prefix}/src/phast/phreeqc/Sun &&\
# src/phasthdf directory
  /usr/bin/find ${instdir}${prefix}/src/phastinput/. -name "*.java" -exec /usr/bin/unix2dos {} \; && \
# database directory
  /usr/bin/install -m 644 ${objdir}/database/*.dat \
    ${instdir}${prefix}/database/. && \
  /usr/bin/unix2dos ${instdir}${prefix}/database/*.dat && \
# clean up .cvsignore files
  /usr/bin/find ${instdir}${prefix}/src -name .cvsignore -exec /usr/bin/rm -f {} \; && \
# examples directory
  for arg in $EXAMPLES; do
    /usr/bin/cp -r ${objdir}/examples/${arg}/* ${instdir}${prefix}/examples/${arg}/.
  done && \
  /usr/bin/find ${instdir}${prefix}/examples/. -type f -exec /usr/bin/unix2dos {} \; && \
  /usr/bin/find ${instdir}${prefix}/examples/. -type f -exec /usr/bin/chmod 644 {} \; && \
# phastexport
  /usr/bin/install -m 755 ${objdir}/src/phasthdf/win32/Release/phasthdf.exe \
    ${instdir}${prefix}/export/bin/phasthdf.exe && \
  /usr/bin/install -m 644 ${objdir}/src/phasthdf/dist/*.jar \
    ${instdir}${prefix}/export/lib && \
  /usr/bin/install -m 755 ${objdir}/src/phasthdf/dist/Win32/*.dll \
    ${instdir}${prefix}/export/lib/Win32 && \
# ModelViewer root directory
  /usr/bin/install -m 644 ${objdir}/Redist/notice.txt \
    ${instdir}${prefix}/mv/. && \
  /usr/bin/install -m 644 ${objdir}/Redist/readme.txt \
    ${instdir}${prefix}/mv/. && \
# ModelViewer doc directory
  /usr/bin/install -m 644 ${objdir}/Redist/ofr02-106.pdf \
    ${instdir}${prefix}/mv/doc/. && \
# ModelViewer bin directory
  /usr/bin/install -m 644 ${objdir}/Redist/modview.chm \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 644 ${objdir}/Redist/DFORRT.DLL \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/hdf5dll.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 644 ${objdir}/Redist/lf90.eer \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/lf90wiod.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Mv/ModelViewer/Release/*.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Mv/Modflow2000Reader1/mf2k_r1.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Mv/Modflow96Reader1/mf96_r1.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Mv/Mt3dmsReader1/mt_r1.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Mv/ModelViewer/Release/modview.exe \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/vtkCommon.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/vtkFiltering.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/vtkGraphics.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/vtkImaging.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/vtkRendering.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/zlib1.dll \
    ${instdir}${prefix}/mv/bin/. && \
  /usr/bin/install -m 755 ${objdir}/Redist/szlibdll.dll \
    ${instdir}${prefix}/mv/bin/. && \
# InstallShield compile
  "${IS_COMPILER}" "${IS_RULFILES}" -I"${IS_INCLUDEIFX}" -I"${IS_INCLUDEISRT}" \
    -I"${IS_INCLUDESCRIPT}" "${IS_LINKPATH1}" "${IS_LINKPATH2}" ${IS_LIBRARIES} \
    ${IS_DEFINITIONS} ${IS_SWITCHES} && \
# InstallShield build
  "${IS_BUILDER}" -p"${IS_INSTALLPROJECT}" -m"${IS_CURRENTBUILD}" && \
# logs
  /usr/bin/install -m 755 ${objdir}/src/phasthdf/win32/phastexport.plg \
    ${instdir}/logs/. && \
  /usr/bin/install -m 644 ${objdir}/phastinput.log \
    ${instdir}/logs/. && \
  /usr/bin/install -m 644 ${objdir}/phast-ser.log \
    ${instdir}/logs/. && \
  /usr/bin/install -m 644 ${objdir}/phast-merge.log \
    ${instdir}/logs/. && \
  /usr/bin/install -m 644 ${objdir}/Mv/mv/mv.plg \
    ${instdir}/logs/. && \
  /usr/bin/install -m 644 "${objdir}/packages/win32-is/Media/SingleDisk/Log Files/"* \
  ${instdir}/logs/. && \
  /usr/bin/install -m 644 "${objdir}/packages/win32-is/Media/SingleDisk/Report Files/"* \
  ${instdir}/logs/. && \
# the installation executable
  /usr/bin/install -m 755 "${objdir}/packages/win32-is/Media/SingleDisk/Disk Images/Disk1/setup.exe" \
    ${instdir}/${FULLPKG}.exe &&\
  run_examples )
}

run_examples() {
  (EXAMPLES="${instdir}${prefix}/examples/ex1 \
  ${instdir}${prefix}/examples/ex4"
  for arg in $EXAMPLES; do
    cd ${arg}
    export TD="`cygpath -w -s "${instdir}${prefix}"`" && \
    ${instdir}${prefix}/bin/${PKG}.bat `basename ${arg}`;
    if [ $? != 0 ] ; then
      exit $?;
    fi
  done )
}


strip() {
  (cd ${instdir} && \
  echo No need to strip )
}

pkg() {
  (cd ${instdir} && \
  tar cvjf ${bin_pkg} * )
}

mkpatch() {
  (cd ${srcdir} && \
  tar xvzf ${src_orig_pkg} && \
  cd ${srcdir}/${BASEPKG} && \
  tar xvzf ${src_orig_pkg_mv} && \
  cd ${srcdir} && \
  mv ${BASEPKG} ../${BASEPKG}-orig && \
  cd ${topdir} && \
  diff -urN -x '.build' -x '.inst' -x '.sinst' \
    ${DIFF_IGNORE} \
    ${BASEPKG}-orig ${BASEPKG} > \
    ${srcinstdir}/${src_patch_name} ; \
  rm -rf ${BASEPKG}-orig )}
spkg() {
  (mkpatch && \
  cp -al ${src_orig_pkg} ${srcinstdir}/${src_orig_pkg_name} && \
  cp -al ${src_orig_pkg_mv} ${srcinstdir}/. && \
  cp -al $0 ${srcinstdir}/`basename $0` && \
  cp Makefile ${srcinstdir}/. && \
  cd ${srcinstdir} && \
  tar cvjf ${src_pkg} * )
}

finish() {
  rm -rf ${srcdir} 
}

case $1 in
  precheck)     precheck ; STATUS=$? ;;
  checkout)     checkout ; STATUS=$? ;;
  dbuild)       dbuild   ; STATUS=$? ;;
  dmkpatch)     dmkpatch ; STATUS=$? ;;
  prep)	        prep     ; STATUS=$? ;;
  conf)	        conf     ; STATUS=$? ;;
  build)        build    ; STATUS=$? ;;
  check)        check    ; STATUS=$? ;;
  clean)        clean    ; STATUS=$? ;;
  install)      install  ; STATUS=$? ;;
  strip)        strip    ; STATUS=$? ;;
  package)      pkg      ; STATUS=$? ;;
  pkg)          pkg      ; STATUS=$? ;;
  mkpatch)      mkpatch  ; STATUS=$? ;;
  src-package)  spkg     ; STATUS=$? ;;
  spkg)         spkg     ; STATUS=$? ;;
  finish)       finish   ; STATUS=$? ;;
  cvsexport)    cvsexport; STATUS=$? ;;
  svnexport)    svnexport; STATUS=$? ;;
  run)          run_examples;  STATUS=$? ;;
  upto-conf)    precheck && prep && conf; STATUS=$? ;;
  upto-build)   precheck && prep && conf && build; STATUS=$? ;;
  upto-install) precheck && prep && conf && build && install; STATUS=$? ;;
  upto-pkg)     precheck && prep && conf && build && install && \
			strip && pkg ; \
			STATUS=$? ;;    
  upto-spkg)    precheck && prep && conf && build && install && \
			strip && pkg && spkg ; \
			STATUS=$? ;;    
  all) precheck && prep && conf && build && install && \
       strip && pkg && spkg && finish ; \
       STATUS=$? ;;
  sofar)  precheck && prep && conf && build && install && \
       STATUS=$? ;;
  *) echo "Error: bad arguments" ; exit 1 ;;
esac
exit ${STATUS}
