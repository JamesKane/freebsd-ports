# Describe DRM devices that are not on PCI, whose kernel bus ID is
# "platform:<name>", as platform devices; Mesa's EGL needs them to be
# described.  Do not return device nodes that do not exist.  Read a node's
# bus ID from dev.drm.N.busid.
#
--- xf86drm.c.orig
+++ xf86drm.c
@@ -3494,6 +3494,11 @@
     id -= drmGetMinorBase(nodetype);
     snprintf(name, sizeof(name), DRM_DIR_NAME "/%s%d", mname,
          id + drmGetMinorBase(type));
+
+    /* Not every device has every node type, e.g. KMS-only drivers have no
+     * render node. Report the node only if it exists, as on Linux. */
+    if (stat(name, &sbuf) != 0)
+        return NULL;
 
     return strdup(name);
 #else
@@ -3617,7 +3622,66 @@
     return -EINVAL;
 }
 #endif
+
+#ifdef __FreeBSD__
+/*
+ * The bus ID the kernel gives the device of a node: dev.drm.N.busid for
+ * /dev/drm/N.  Older kernels only have hw.dri.M.busid, one per device, where
+ * card N and renderD(128+N) are guessed to be hw.dri.N.
+ */
+static int get_sysctl_busid(int maj, int min, char *busid, size_t len)
+{
+    char dname[SPECNAMELEN];
+    char sysctl_name[32];
+    size_t size;
+    int id, type;
+    unsigned int rdev;
 
+    rdev = makedev(maj, min);
+    if (!devname_r(rdev, S_IFCHR, dname, sizeof(dname)))
+      return -EINVAL;
+
+    if (sscanf(dname, "drm/%d\n", &id) != 1)
+        return -EINVAL;
+
+    snprintf(sysctl_name, sizeof(sysctl_name), "dev.drm.%d.busid", id);
+    size = len;
+    if (sysctlbyname(sysctl_name, busid, &size, NULL, 0) == 0)
+        return 0;
+
+    type = drmGetMinorType(maj, min);
+    if (type == DRM_NODE_RENDER)
+        id -= 128;
+    else if (type != DRM_NODE_PRIMARY)
+        return -EINVAL;
+    if (id < 0)
+        return -EINVAL;
+    snprintf(sysctl_name, sizeof(sysctl_name), "hw.dri.%d.busid", id);
+    size = len;
+    if (sysctlbyname(sysctl_name, busid, &size, NULL, 0))
+      return -EINVAL;
+
+    return 0;
+}
+
+/*
+ * Drivers of devices that are not on PCI give the bus ID "platform:<name>":
+ * get the name, if the node's device is one.
+ */
+static int get_sysctl_platform_name(int maj, int min, char *name, size_t len)
+{
+    static const char prefix[] = "platform:";
+    char busid[256];
+
+    if (get_sysctl_busid(maj, min, busid, sizeof(busid)) ||
+        strncmp(busid, prefix, strlen(prefix)) != 0)
+        return -ENOENT;
+    strncpy(name, busid + strlen(prefix), len);
+    name[len - 1] = '\0';
+    return 0;
+}
+#endif
+
 static int drmParseSubsystemType(int maj, int min)
 {
 #ifdef __linux__
@@ -3639,8 +3703,14 @@
             return DRM_BUS_VIRTIO;
      }
     return subsystem_type;
-#elif defined(__OpenBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
+#elif defined(__FreeBSD__)
+    char name[DRM_PLATFORM_DEVICE_NAME_LEN];
+
+    if (get_sysctl_platform_name(maj, min, name, sizeof(name)) == 0)
+        return DRM_BUS_PLATFORM;
     return DRM_BUS_PCI;
+#elif defined(__OpenBSD__) || defined(__DragonFly__)
+    return DRM_BUS_PCI;
 #else
 #warning "Missing implementation of drmParseSubsystemType"
     return -EINVAL;
@@ -3668,45 +3738,12 @@
 #ifdef __FreeBSD__
 static int get_sysctl_pci_bus_info(int maj, int min, drmPciBusInfoPtr info)
 {
-    char dname[SPECNAMELEN];
-    char sysctl_name[16];
     char sysctl_val[256];
-    size_t sysctl_len;
-    int id, type, nelem;
-    unsigned int rdev, majmin, domain, bus, dev, func;
+    int nelem;
+    unsigned int domain, bus, dev, func;
 
-    rdev = makedev(maj, min);
-    if (!devname_r(rdev, S_IFCHR, dname, sizeof(dname)))
-      return -EINVAL;
-
-    if (sscanf(dname, "drm/%d\n", &id) != 1)
-        return -EINVAL;
-    type = drmGetMinorType(maj, min);
-    if (type == -1)
-        return -EINVAL;
-
-    /* BUG: This above section is iffy, since it mandates that a driver will
-     * create both card and render node.
-     * If it does not, the next DRM device will create card#X and
-     * renderD#(128+X)-1.
-     * This is a possibility in FreeBSD but for now there is no good way for
-     * obtaining the info.
-     */
-    switch (type) {
-    case DRM_NODE_PRIMARY:
-         break;
-    case DRM_NODE_RENDER:
-         id -= 128;
-         break;
-    }
-    if (id < 0)
-        return -EINVAL;
-
-    if (snprintf(sysctl_name, sizeof(sysctl_name), "hw.dri.%d.busid", id) <= 0)
+    if (get_sysctl_busid(maj, min, sysctl_val, sizeof(sysctl_val)))
       return -EINVAL;
-    sysctl_len = sizeof(sysctl_val);
-    if (sysctlbyname(sysctl_name, sysctl_val, &sysctl_len, NULL, 0))
-      return -EINVAL;
 
     #define bus_fmt "pci:%04x:%02x:%02x.%u"
 
@@ -4319,6 +4356,10 @@
     free(name);
 
     return 0;
+#elif defined(__FreeBSD__)
+    /* Like Linux without OF data: the name from the bus ID. */
+    return get_sysctl_platform_name(maj, min, fullname,
+                                    DRM_PLATFORM_DEVICE_NAME_LEN);
 #else
 #warning "Missing implementation of drmParseOFBusInfo"
     return -EINVAL;
@@ -4379,6 +4420,25 @@
 
     free(*compatible);
     return err;
+#elif defined(__FreeBSD__)
+    char name[DRM_PLATFORM_DEVICE_NAME_LEN];
+    int err;
+
+    /* Like Linux without OF data: one entry, the device's name. */
+    err = drmParseOFBusInfo(maj, min, name);
+    if (err)
+        return err;
+
+    *compatible = calloc(2, sizeof(char *));
+    if (!*compatible)
+        return -ENOMEM;
+    (*compatible)[0] = strdup(name);
+    if (!(*compatible)[0]) {
+        free(*compatible);
+        return -ENOMEM;
+    }
+
+    return 0;
 #else
 #warning "Missing implementation of drmParseOFDeviceInfo"
     return -EINVAL;
