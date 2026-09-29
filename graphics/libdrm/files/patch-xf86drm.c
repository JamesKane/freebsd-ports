# Describe DRM devices that are not on PCI, whose kernel bus ID is
# "platform:<name>", as platform devices; Mesa's EGL needs them to be
# described.  Do not return device nodes that do not exist.  Find the hw.dri
# slot of a node by its number.
#
--- xf86drm.c.orig
+++ xf86drm.c
@@ -3495,6 +3495,11 @@
     snprintf(name, sizeof(name), DRM_DIR_NAME "/%s%d", mname,
          id + drmGetMinorBase(type));
 
+    /* Not every device has every node type, e.g. KMS-only drivers have no
+     * render node. Report the node only if it exists, as on Linux. */
+    if (stat(name, &sbuf) != 0)
+        return NULL;
+
     return strdup(name);
 #else
     struct stat sbuf;
@@ -3617,7 +3622,69 @@
     return -EINVAL;
 }
 #endif
+
+#ifdef __FreeBSD__
+/*
+ * The hw.dri slot of the device of node "id" of the given type.  Kernels
+ * that give each slot the numbers of its nodes are searched; for older ones,
+ * guess that card N and renderD(128+N) are hw.dri.N.
+ */
+static int get_sysctl_slot(int id, int type)
+{
+    char sysctl_name[32];
+    const char *leaf;
+    size_t len;
+    int i, node, found;
+
+    leaf = type == DRM_NODE_RENDER ? "render" : "primary";
+    found = 0;
+    for (i = 0; i < 10; i++) {
+        snprintf(sysctl_name, sizeof(sysctl_name), "hw.dri.%d.%s", i, leaf);
+        len = sizeof(node);
+        if (sysctlbyname(sysctl_name, &node, &len, NULL, 0) != 0)
+            continue;
+        if (node == id)
+            return i;
+        found = 1;
+    }
+    if (found)
+        return -1;
+    return type == DRM_NODE_RENDER ? id - 128 : id;
+}
 
+/* The bus ID the kernel gives the device of a node, from hw.dri.N.busid. */
+static int get_sysctl_busid(int maj, int min, char *busid, size_t len)
+{
+    char dname[SPECNAMELEN];
+    char sysctl_name[16];
+    int id, type;
+    unsigned int rdev;
+
+    rdev = makedev(maj, min);
+    if (!devname_r(rdev, S_IFCHR, dname, sizeof(dname)))
+      return -EINVAL;
+
+    if (sscanf(dname, "drm/%d\n", &id) != 1)
+        return -EINVAL;
+    type = drmGetMinorType(maj, min);
+    if (type == -1)
+        return -EINVAL;
+
+    id = get_sysctl_slot(id, type);
+    if (id < 0)
+        return -EINVAL;
+
+    if (snprintf(sysctl_name, sizeof(sysctl_name), "hw.dri.%d.busid", id) <= 0)
+      return -EINVAL;
+    if (sysctlbyname(sysctl_name, busid, &len, NULL, 0))
+      return -EINVAL;
+
+    return 0;
+}
+
+#define FREEBSD_PLATFORM_BUSID "platform:"
+#endif
+
 static int drmParseSubsystemType(int maj, int min)
 {
 #ifdef __linux__
@@ -3639,7 +3706,16 @@
             return DRM_BUS_VIRTIO;
      }
     return subsystem_type;
-#elif defined(__OpenBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
+#elif defined(__FreeBSD__)
+    char busid[256];
+
+    /* Drivers of devices that are not on PCI use "platform:<name>". */
+    if (get_sysctl_busid(maj, min, busid, sizeof(busid)) == 0 &&
+        strncmp(busid, FREEBSD_PLATFORM_BUSID,
+                strlen(FREEBSD_PLATFORM_BUSID)) == 0)
+        return DRM_BUS_PLATFORM;
+    return DRM_BUS_PCI;
+#elif defined(__OpenBSD__) || defined(__DragonFly__)
     return DRM_BUS_PCI;
 #else
 #warning "Missing implementation of drmParseSubsystemType"
@@ -3668,44 +3744,11 @@
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
-      return -EINVAL;
-    sysctl_len = sizeof(sysctl_val);
-    if (sysctlbyname(sysctl_name, sysctl_val, &sysctl_len, NULL, 0))
+    if (get_sysctl_busid(maj, min, sysctl_val, sizeof(sysctl_val)))
       return -EINVAL;
 
     #define bus_fmt "pci:%04x:%02x:%02x.%u"
@@ -4319,6 +4362,20 @@
     free(name);
 
     return 0;
+#elif defined(__FreeBSD__)
+    char busid[256];
+
+    /* Like Linux without OF data: the name from "platform:<name>". */
+    if (get_sysctl_busid(maj, min, busid, sizeof(busid)) ||
+        strncmp(busid, FREEBSD_PLATFORM_BUSID,
+                strlen(FREEBSD_PLATFORM_BUSID)) != 0)
+        return -ENOENT;
+
+    strncpy(fullname, busid + strlen(FREEBSD_PLATFORM_BUSID),
+            DRM_PLATFORM_DEVICE_NAME_LEN);
+    fullname[DRM_PLATFORM_DEVICE_NAME_LEN - 1] = '\0';
+
+    return 0;
 #else
 #warning "Missing implementation of drmParseOFBusInfo"
     return -EINVAL;
@@ -4379,6 +4436,25 @@
 
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
