LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=6b566c5b4da35d474758324899cb4562"

SRC_URI = "git://anongit.freedesktop.org/beignet;nobranch=1 \
           file://fix-llvm-paths.patch \
           file://0001-api-mark-platform-not-supported-if-device-is-not-fou.patch \
           "
SRC_URI_append_class-native = " file://0001-reduced-native-for-1.3.1.patch"
SRC_URI_append_class-target = " file://0001-Run-native-gbe_bin_generater-to-compile-built-in-ker.patch"

# CMake cannot digest "+" in pathes -> replace it with dots.
PV = "1.3.1.${@ 'git${SRCPV}'.replace('+', '.')}"
SRCREV = "6804cca263edd11fd03b2d7f5b7ba034d9a013c3"
S = "${WORKDIR}/git"

# we need to depend on ocl-icd, so that the exported symbols go right
DEPENDS = "clang libdrm mesa ocl-icd ncurses"
DEPENDS_append_class-target = " beignet-native"
DEPENDS_class-native = "clang-native"

# To solve the PCI_ID problem, we use the target from filename for
# installing the library.

CL_HW_TARGET ?= "${@ "" if d.getVar('PN').endswith('-native') else d.getVar('PN').split('-')[1]}"

inherit cmake pkgconfig pythonnative

EXTRA_OECMAKE = " -DSTANDALONE_GBE_COMPILER_DIR=${STAGING_BINDIR_NATIVE} -DLLVM_LIBRARY_DIR=${STAGING_LIBDIR} -DBEIGNET_INSTALL_DIR=${libdir}/beignet-${CL_HW_TARGET}"
EXTRA_OECMAKE_class-native = " -DBEIGNET_INSTALL_DIR=/usr/lib/beignet -DLLVM_LIBRARY_DIR=${STAGING_LIBDIR_NATIVE}"

# TODO respect distrofeatures for x11
PACKAGECONFIG ??= ""
PACKAGECONFIG[examples] = '-DBUILD_EXAMPLES=1,-DBUILD_EXAMPLES=0,libva'
# TODO: add explicit on/off upstream
PACKAGECONFIG[x11] = ",,libxext libxfixes"

FILES_${PN} += " \
                ${sysconfdir}/OpenCL/vendors/intel-beignet-${CL_HW_TARGET}.icd \
                ${libdir} \
                ${libdir}/beignet-${CL_HW_TARGET}/ \
                ${libdir}/beignet-${CL_HW_TARGET}/* \
               "

do_install_append_class-target () {
    # Change the intel-beignet.icd file
    rm ${D}${sysconfdir}/OpenCL/vendors/intel-beignet.icd
    echo ${libdir}/beignet-${CL_HW_TARGET}/libcl.so > ${D}${sysconfdir}/OpenCL/vendors/intel-beignet-${CL_HW_TARGET}.icd

    # Remove AppStream metadata so that the packages won't conflict
    rm ${D}${datadir}/metainfo/com.intel.beignet.metainfo.xml
    rmdir --ignore-fail-on-non-empty -p ${D}${datadir}/metainfo

    rm -r ${D}${includedir}
}

do_install_class-native() {
    install -d ${D}${libdir}/cmake
    install -m644 ${S}/CMake/FindStandaloneGbeCompiler.cmake ${D}${libdir}/cmake

    install -d ${D}${bindir}
    install ${B}/backend/src/gbe_bin_generater ${D}${bindir}
    install ${B}/backend/src/libgbe.so ${D}${libdir}

    install -d ${D}${bindir}/include
    install ${B}/backend/src/libocl/usr/lib/beignet/include/* ${D}${bindir}/include
    install ${B}/backend/src/libocl/usr/lib/beignet/beignet.bc ${D}${bindir}/
}
