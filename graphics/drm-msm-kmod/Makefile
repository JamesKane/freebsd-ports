PORTNAME=	drm-msm-kmod
DISTVERSION=	0.0.20260929
PORTREVISION=	3
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
GH_TAGNAME=	02f48c0
GH_TUPLE=	freebsd:drm-kmod:${DRM_KMOD_GH_TAGNAME}:drm

.include "${.CURDIR}/../drm-latest-kmod/Makefile.version"

SUB_FILES=	pkg-message
SUB_LIST=	OPSYS=${OPSYS} OSREL=${OSREL}

MAKE_ENV+=	DRMKMOD=${WRKSRC_drm} \
		MAKEOBJDIRPREFIX=${WRKSRC}/obj

PLIST_FILES=	${KMODDIR}/msm.ko

pre-build:
	${MKDIR} ${WRKSRC}/obj
	(cd ${WRKSRC} ; ${SETENV} ${MAKE_ENV} ${MAKE_CMD} obj)

.include <bsd.port.mk>
