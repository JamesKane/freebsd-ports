# FreeBSD has no ENODATA; use ENOATTR, as src/util/os_file.c notes.
#
--- src/freedreno/drm/msm/msm_bo.c.orig
+++ src/freedreno/drm/msm/msm_bo.c
@@ -8,6 +8,10 @@
 
 #include "msm_priv.h"
 
+#ifndef ENODATA
+#define ENODATA ENOATTR
+#endif
+
 static int
 bo_allocate(struct msm_bo *msm_bo)
 {
