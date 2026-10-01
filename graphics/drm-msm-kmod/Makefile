PORTNAME=	drm-msm-kmod
DISTVERSION=	0.0.20260929
PORTREVISION=	15
CATEGORIES=	graphics kld

MAINTAINER=	kanej1973@gmail.com
COMMENT=	DRM driver for Qualcomm Adreno GPUs (msm)
WWW=		https://github.com/JamesKane/drm-msm-kmod

LICENSE=	BSD2CLAUSE GPLv2 MIT
LICENSE_COMB=	multi

ONLY_FOR_ARCHS=	aarch64
ONLY_FOR_ARCHS_REASON=	Qualcomm Adreno GPUs are only supported on aarch64

RUN_DEPENDS=	${KMODDIR}/drm.ko:graphics/drm-latest-kmod \
		gpu-firmware-qcom-kmod-a660>0:graphics/gpu-firmware-qcom-kmod@a660 \
		gpu-firmware-qcom-kmod-sc8280xp>0:graphics/gpu-firmware-qcom-kmod@sc8280xp

USES=		kmod uidfix

USE_GITHUB=	yes
GH_ACCOUNT=	JamesKane
GH_TAGNAME=	0f23f29
GH_TUPLE=	freebsd:drm-kmod:${DRM_KMOD_GH_TAGNAME}:drm

.include "${.CURDIR}/../drm-latest-kmod/Makefile.version"

SUB_FILES=	pkg-message
SUB_LIST=	OPSYS=${OPSYS} OSREL=${OSREL}

MAKE_ENV+=	DRMKMOD=${WRKSRC_drm} \
		MAKEOBJDIRPREFIX=${WRKSRC}/obj

PLIST_FILES=	${KMODDIR}/msm.ko \
		etc/power_profile.d/msm

pre-build:
	${MKDIR} ${WRKSRC}/obj
	(cd ${WRKSRC} ; ${SETENV} ${MAKE_ENV} ${MAKE_CMD} obj)

post-install:
	${MKDIR} ${STAGEDIR}${PREFIX}/etc/power_profile.d
	${INSTALL_SCRIPT} ${FILESDIR}/power_profile.msm \
		${STAGEDIR}${PREFIX}/etc/power_profile.d/msm

.include <bsd.port.mk>
