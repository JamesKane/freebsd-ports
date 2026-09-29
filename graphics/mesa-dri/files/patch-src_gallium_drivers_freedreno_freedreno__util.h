# SYS_gettid is Linux-only; use pthread_getthreadid_np() on FreeBSD.
#
--- src/gallium/drivers/freedreno/freedreno_util.h.orig
+++ src/gallium/drivers/freedreno/freedreno_util.h
@@ -95,12 +95,20 @@
 
 #include <unistd.h>
 #include <sys/types.h>
+#if defined(__linux__)
 #include <sys/syscall.h>
+#define fd_gettid() ((pid_t)syscall(SYS_gettid))
+#elif defined(__FreeBSD__)
+#include <pthread_np.h>
+#define fd_gettid() ((pid_t)pthread_getthreadid_np())
+#else
+#define fd_gettid() getpid()
+#endif
 
 #define DBG(fmt, ...)                                                          \
    do {                                                                        \
       if (FD_DBG(MSGS))                                                        \
-         mesa_logd("%5d: %s:%d: " fmt, ((pid_t)syscall(SYS_gettid)),           \
+         mesa_logd("%5d: %s:%d: " fmt, fd_gettid(),                            \
                                         __func__, __LINE__,                    \
                                         ##__VA_ARGS__);                        \
    } while (0)
