#!/bin/bash
set -ev
: ${SDCC:=3.3.0}
: ${SDCC_DIR:=/usr/local}

if [[ $(sdcc --version) =~ "${SDCC}" ]]; then
  echo "Found sdcc-${SDCC}, build skipped."
  exit 0
fi

if [ ! -d sdcc-${SDCC} ]; then
  if [ ! -f sdcc-src-${SDCC}.tar.bz2 ]; then
    wget "http://downloads.sourceforge.net/project/sdcc/sdcc/${SDCC}/sdcc-src-${SDCC}.tar.bz2" -O sdcc-src-${SDCC}.tar.bz2
  fi
  tar jvxf sdcc-src-${SDCC}.tar.bz2
fi

cd sdcc-${SDCC}
case "${SDCC}" in
3.[34].*)
patch -u -N -p1 <<'__PATCH_LINE__' || let "$?<=1"
Index: sdcc-3.4.0/support/sdbinutils/bfd/bfd-in.h
===================================================================
--- sdcc-3.4.0.orig/support/sdbinutils/bfd/bfd-in.h  2012-11-05 21:26:25.000000000 +0800
+++ sdcc-3.4.0/support/sdbinutils/bfd/bfd-in.h 2014-06-19 16:40:09.274149785 +0800
@@ -294,9 +294,9 @@
 
 #define bfd_is_com_section(ptr) (((ptr)->flags & SEC_IS_COMMON) != 0)
 
-#define bfd_set_section_vma(bfd, ptr, val) (((ptr)->vma = (ptr)->lma = (val)), ((ptr)->user_set_vma = TRUE), TRUE)
-#define bfd_set_section_alignment(bfd, ptr, val) (((ptr)->alignment_power = (val)),TRUE)
-#define bfd_set_section_userdata(bfd, ptr, val) (((ptr)->userdata = (val)),TRUE)
+#define bfd_set_section_vma(bfd, ptr, val) (((ptr)->vma = (ptr)->lma = (val)), ((ptr)->user_set_vma = TRUE))
+#define bfd_set_section_alignment(bfd, ptr, val) ((ptr)->alignment_power = (val))
+#define bfd_set_section_userdata(bfd, ptr, val) ((ptr)->userdata = (val))
 /* Find the address one past the end of SEC.  */
 #define bfd_get_section_limit(bfd, sec) \
   (((bfd)->direction != write_direction && (sec)->rawsize != 0 \
@@ -519,7 +519,7 @@
 
 #define bfd_get_symbol_leading_char(abfd) ((abfd)->xvec->symbol_leading_char)
 
-#define bfd_set_cacheable(abfd,bool) (((abfd)->cacheable = bool), TRUE)
+#define bfd_set_cacheable(abfd,bool) ((abfd)->cacheable = bool)
 
 extern bfd_boolean bfd_cache_close
   (bfd *abfd);
Index: sdcc-3.4.0/support/sdbinutils/bfd/bfd-in2.h
===================================================================
--- sdcc-3.4.0.orig/support/sdbinutils/bfd/bfd-in2.h 2012-11-05 21:26:25.000000000 +0800
+++ sdcc-3.4.0/support/sdbinutils/bfd/bfd-in2.h  2014-06-19 16:42:09.110092421 +0800
@@ -301,9 +301,9 @@
 
 #define bfd_is_com_section(ptr) (((ptr)->flags & SEC_IS_COMMON) != 0)
 
-#define bfd_set_section_vma(bfd, ptr, val) (((ptr)->vma = (ptr)->lma = (val)), ((ptr)->user_set_vma = TRUE), TRUE)
-#define bfd_set_section_alignment(bfd, ptr, val) (((ptr)->alignment_power = (val)),TRUE)
-#define bfd_set_section_userdata(bfd, ptr, val) (((ptr)->userdata = (val)),TRUE)
+#define bfd_set_section_vma(bfd, ptr, val) (((ptr)->vma = (ptr)->lma = (val)), ((ptr)->user_set_vma = TRUE))
+#define bfd_set_section_alignment(bfd, ptr, val) ((ptr)->alignment_power = (val))
+#define bfd_set_section_userdata(bfd, ptr, val) ((ptr)->userdata = (val))
 /* Find the address one past the end of SEC.  */
 #define bfd_get_section_limit(bfd, sec) \
   (((bfd)->direction != write_direction && (sec)->rawsize != 0 \
@@ -526,7 +526,7 @@
 
 #define bfd_get_symbol_leading_char(abfd) ((abfd)->xvec->symbol_leading_char)
 
-#define bfd_set_cacheable(abfd,bool) (((abfd)->cacheable = bool), TRUE)
+#define bfd_set_cacheable(abfd,bool) ((abfd)->cacheable = bool)
 
 extern bfd_boolean bfd_cache_close
   (bfd *abfd);
__PATCH_LINE__
;;
esac

DISABLE_DEVICES="z80 z180 r2k r3ka gbz80 ds390 ds400 pic14 pic16 hc08 s08"
CONFIGURE_OPT="--prefix=${SDCC_DIR} --disable-ucsim --disable-sdcdb --disable-non-free" 
case "${SDCC}" in
3.[45678].*)
DISABLE_DEVICES+=" tlcs90 stm8 stm8";;
3.[9].*)
DISABLE_DEVICES+=" tlcs90 stm8 stm8 ez80_z80 pdk13 pdk14 pdk15";;
esac
for dev in ${DISABLE_DEVICES}; do
  CONFIGURE_OPT+=" --disable-${dev}-port";
done

./configure ${CONFIGURE_OPT} \
&& make -j \
&& make install