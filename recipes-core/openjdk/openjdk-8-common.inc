SUMMARY = "Java runtime based upon the OpenJDK Project"
HOMEPAGE = "http://openjdk.java.net/"
LICENSE  = "GPL-2.0-with-classpath-exception"

LIC_FILES_CHKSUM = "file://LICENSE;md5=3e0b59f8fac05c3c03d4a26bbda13f8f"

require openjdk-common.inc

AUTOTOOLS_SCRIPT_PATH = "${S}/common/autoconf/"
export AUTOCONF_DIR="${AUTOTOOLS_SCRIPT_PATH}"
FILESPATH =. "${FILE_DIRNAME}/patches-openjdk-8:"

# for weird openjdk-common.inc
S = "${WORKDIR}/${OPENJDK_HG_U}-${OPENJDK_CHANGESET}"

SRC_URI = "\
    ${OPENJDK_URI} \
    ${HOTSPOT_URI} \
    ${CORBA_URI} \
    ${JAXP_URI} \
    ${JAXWS_URI} \
    ${JDK_URI} \
    ${LANGTOOLS_URI} \
    ${NASHORN_URI} \
    file://LICENSE \
    ${PATCHES_URI} \
"

do_configure_prepend () {
    export TOPDIR=${S}
}

do_unpack_extract_submodules () {
    cd "${S}"
    # tar --transform
    tar xjf ${WORKDIR}/${CORBA_FILE_LOCAL} --transform "s,-${CORBA_CHANGESET},,g"
    tar xjf ${WORKDIR}/${HOTSPOT_FILE_LOCAL} --transform "s,-${HOTSPOT_CHANGESET},,g"
    tar xjf ${WORKDIR}/${JAXP_FILE_LOCAL} --transform "s,-${JAXP_CHANGESET},,g"
    tar xjf ${WORKDIR}/${JAXWS_FILE_LOCAL} --transform "s,-${JAXWS_CHANGESET},,g"
    tar xjf ${WORKDIR}/${JDK_FILE_LOCAL} --transform "s,-${JDK_CHANGESET},,g"
    tar xjf ${WORKDIR}/${LANGTOOLS_FILE_LOCAL} --transform "s,-${LANGTOOLS_CHANGESET},,g"
    tar xjf ${WORKDIR}/${NASHORN_FILE_LOCAL} --transform "s,-${NASHORN_CHANGESET},,g"
}

# I copied this from above and adjusted the path suitable for archiver.bbclass
# This will be called by some hacked copy of do_unpack_and_patch
# which originates from archiver.bbclass
# the hacked version resides in this file
do_unpack_extract_submodules_archiver () {
    cd "${S}"
    tar xjf ${WORKDIR}/../${CORBA_FILE_LOCAL} --transform "s,-${CORBA_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${HOTSPOT_FILE_LOCAL} --transform "s,-${HOTSPOT_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${JAXP_FILE_LOCAL} --transform "s,-${JAXP_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${JAXWS_FILE_LOCAL} --transform "s,-${JAXWS_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${JDK_FILE_LOCAL} --transform "s,-${JDK_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${LANGTOOLS_FILE_LOCAL} --transform "s,-${LANGTOOLS_CHANGESET},,g"
    tar xjf ${WORKDIR}/../${NASHORN_FILE_LOCAL} --transform "s,-${NASHORN_CHANGESET},,g"
}


do_unpack_delete_X11_wrappers() {
    find ${S}/jdk/src/solaris/classes/sun/awt/X11 -maxdepth 1 -name '*.java' -delete
}

def package_config_option_cleanup(d):
    distro_x11 = bb.utils.contains('DISTRO_FEATURES', 'x11', True, False, d)
    distro_alsa = bb.utils.contains('DISTRO_FEATURES', 'alsa', True, False, d)
    distro_pulseaudio = bb.utils.contains('DISTRO_FEATURES', 'pulseaudio', True, False, d)

    with_x11 = bb.utils.contains('PACKAGECONFIG', 'x11', True, False, d)
    with_cups = bb.utils.contains('PACKAGECONFIG', 'cups', True, False, d)
    with_alsa = bb.utils.contains('PACKAGECONFIG', 'alsa', True, False, d)
    with_pulseaudio = bb.utils.contains('PACKAGECONFIG', 'pulseaudio', True, False, d)

    option_headless = bb.utils.contains('PACKAGECONFIG', 'headless', True, False, d)
    option_headful = bb.utils.contains('PACKAGECONFIG', 'headful', True, False, d)
    option_soundless = bb.utils.contains('PACKAGECONFIG', 'soundless', True, False, d)

    if option_headless and option_headful:
        option_headless = False

    if option_headful and not with_x11:
        with_x11 = True

    if option_headful and not with_cups:
        with_cups = True

    if option_soundless and with_alsa:
        with_alsa = False

    if option_soundless and with_pulseaudio:
        with_pulseaudio = False

    if with_x11 and not distro_x11:
        with_x11 = False

    if with_alsa and not distro_alsa:
        with_alsa = False

    if with_pulseaudio and not distro_pulseaudio:
        with_pulseaudio = False

    if option_headful and not with_x11:
        option_headful = False

    if option_headful and not with_cups:
        option_headful = False

    if not option_headless and not with_x11:
        option_headless = True

    if not option_headless and not with_cups:
        option_headless = True

    if not option_soundless and not with_alsa:
        option_soundless = True

    if not option_soundless and not with_pulseaudio:
        option_soundless = True

    options = {'make': [], 'cpp': [], 'env': [], 'configure': ["--disable-ccache"] }

    if option_headful:
        options['configure'].append("--enable-headful")

    if option_headless:
        options['configure'].append("--disable-headful")
        options['make'].append("BUILD_HEADLESS_ONLY=1")
        options['make'].append("BUILD_HEADLESS=true")
        options['cpp'].append("-DHEADLESS=true")

    if option_soundless:
        options['make'].append("BUILD_SOUNDLESS_ONLY=1")

    if not with_x11:
        options['make'].append("X11_NOT_NEEDED=1")
        options['env'].append("X11_NOT_NEEDED=yes")

    if not with_cups:
        options['make'].append("CUPS_NOT_NEEDED=1")
        options['env'].append("CUPS_NOT_NEEDED=yes")

    if not with_alsa:
        options['make'].append("ALSA_NOT_NEEDED=1")
        options['env'].append("ALSA_NOT_NEEDED=yes")

    if not with_pulseaudio:
        options['make'].append("PULSE_NOT_NEEDED=1")
        options['env'].append("PULSE_NOT_NEEDED=yes")

    options = [ ' '.join(options['make']), ' '.join(options['cpp']), ' '.join(options['env']), ' '.join(options['configure']) ]

    return options

def jdk_make_options(d):
    options = package_config_option_cleanup(d)
    return options[0]

def jdk_cpp_options(d):
    options = package_config_option_cleanup(d)
    if not options[1]:
        return ""

    return " " + options[1]

def jdk_environment_options(d):
    options = package_config_option_cleanup(d)
    return options[2]

def jdk_configure_options(d):
    options = package_config_option_cleanup(d)
    return options[3]

do_unpack[postfuncs] += "do_unpack_extract_submodules"
do_unpack[postfuncs] += "${@bb.utils.contains('PACKAGECONFIG', 'x11', '', 'do_unpack_delete_X11_wrappers', d)}"

export DEBUG_BINARIES = "true"

ALTERNATIVE_PRIORITY = "50"

OPENJDK_UPDATE_VERSION = "${PV}"
OPENJDK_BUILD_NUMBER = "ga"
EXTRA_OECONF_append = "\
        --with-build-number=${OPENJDK_BUILD_NUMBER} \
        --with-update-version=${OPENJDK_UPDATE_VERSION} \
"

# Since v6, GCC sets the default C++ standard to C++14 and introduces
# dead store elimination by default. OpenJDK 8 is not ready for either
# of these changes.
FLAGS_GCC6 = "-fno-lifetime-dse -fno-delete-null-pointer-checks"
FLAGS_GCC7 = "-fno-lifetime-dse -fno-delete-null-pointer-checks"
FLAGS_GCC8 = "-fno-lifetime-dse -fno-delete-null-pointer-checks"
FLAGS_GCC9 = "-fno-lifetime-dse -fno-delete-null-pointer-checks -Wno-error=format-overflow"

# flags for -native, and for bits that need a host-tool during -cross
BUILD_CFLAGS_append = " ${@openjdk_build_helper_get_build_cflags(d)}"
BUILD_CXXFLAGS_append = " ${@openjdk_build_helper_get_build_cflags(d)}"
# flags for -cross
TARGET_CFLAGS_append = " ${@openjdk_build_helper_get_target_cflags(d)}"
TARGET_CXXFLAGS_append = " ${@openjdk_build_helper_get_target_cflags(d)}"

# Hacked copy of do_unpack_and_patch from archiver.bbclass
# !!! Note:
#     This might need an updated every time do_unpack_and_patch
#     changes in archiver.bbclass !!!
#     This version is from zeus.
#
# What archiver.bblass does among other things:
#   If we include the archiver.bbclass and depending on what we configured
#   do_unpack_and_patch is always used for ARCHIVER_MODE[src] = "patched"
#   and ARCHIVER_MODE[src] = "configured".
#   For ARCHIVER_MODE[src] = "original" it is only enabled if
#   ARCHIVER_MODE[diff] = "1" is also enabled.
#   for the new ARCHIVE_MODE[src] = "mirror" it is never enabled.
#
#   do_unpack_and_patch() sets WORKDIR to ${ARCHIVER_WORKDIR},
#   which defaults to ${WORKDIR}/archiver-work/ and also changes S.

# The issue here:
#   This makes the hacky do_unpack_extract_submodules in this file
#   unsuitable to be used with archiver.bbclass.
#   The tarballs ${WORKDIR}/${XXX_XXX_LOCAL} can not be found anymore
#   so the source code which should be patched is not available anymore
#   which leads so a quilt error.
#
# The workaround:
#   do_unpack_and_patch() was copied from the archiver.bbclass and hacked
#
#   1) do_unpack_extract_submodules_archiver is a version of
#      do_unpack_extract_submodules adjusted for the archiver.bbclass
#
#   2) do_unpack_extract_submodules_archiver needs to be called after
#      do_unpack and before do_patch, which is difficult without
#      changing do_unpack_and_patch, so I hacked it here.
#      Note that this is only a one-line change to do_unpack_and_patch.
#      bb.build.exec_func('do_unpack_extract_submodules_archiver',d)
#
# Run do_unpack and do_patch
python do_unpack_and_patch() {
    # bb.warn ("--> Hello from my do_unpack_and_patch <--")
    if d.getVarFlag('ARCHIVER_MODE', 'src') not in \
            [ 'patched', 'configured'] and \
            d.getVarFlag('ARCHIVER_MODE', 'diff') != '1':
        return
    ar_outdir = d.getVar('ARCHIVER_OUTDIR')
    ar_workdir = d.getVar('ARCHIVER_WORKDIR')
    ar_sysroot_native = d.getVar('STAGING_DIR_NATIVE')
    pn = d.getVar('PN')

    # The kernel class functions require it to be on work-shared, so we dont change WORKDIR
    if not is_work_shared(d):
        # Change the WORKDIR to make do_unpack do_patch run in another dir.
        d.setVar('WORKDIR', ar_workdir)
        # Restore the original path to recipe's native sysroot (it's relative to WORKDIR).
        d.setVar('STAGING_DIR_NATIVE', ar_sysroot_native)

        # The changed 'WORKDIR' also caused 'B' changed, create dir 'B' for the
        # possibly requiring of the following tasks (such as some recipes's
        # do_patch required 'B' existed).
        bb.utils.mkdirhier(d.getVar('B'))

        bb.build.exec_func('do_unpack', d)
        # hacked function which extracts stuff into dirs for archiver
        bb.build.exec_func('do_unpack_extract_submodules_archiver',d)

    # Save the original source for creating the patches
    if d.getVarFlag('ARCHIVER_MODE', 'diff') == '1':
        src = d.getVar('S').rstrip('/')
        src_orig = '%s.orig' % src
        oe.path.copytree(src, src_orig)

    # Make sure gcc and kernel sources are patched only once
    if not (d.getVar('SRC_URI') == "" or is_work_shared(d)):
        bb.build.exec_func('do_patch', d)

    # Create the patches
    if d.getVarFlag('ARCHIVER_MODE', 'diff') == '1':
        bb.note('Creating diff gz...')
        create_diff_gz(d, src_orig, src, ar_outdir)
        bb.utils.remove(src_orig, recurse=True)
}
