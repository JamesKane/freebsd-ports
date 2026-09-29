PORTNAME=	gpu-firmware-qcom-kmod
PORTVERSION=	20260929
CATEGORIES=	graphics

MAINTAINER=	x11@FreeBSD.org
COMMENT=	Firmware modules for ${FLAVOR} Qualcomm Adreno GPUs
WWW=		https://github.com/freebsd/drm-kmod-firmware

LICENSE=	QCOM
LICENSE_NAME=	Qualcomm Technologies redistributable firmware license
LICENSE_FILE=	${WRKSRC}/LICENSE.qcom
LICENSE_PERMS=	dist-mirror pkg-mirror auto-accept

ONLY_FOR_ARCHS=		aarch64
ONLY_FOR_ARCHS_REASON=	Qualcomm Adreno GPUs are only supported on aarch64

PKGNAMESUFFIX=	-${FLAVOR:C/_/-/g}
FLAVORS=	a660 \
		sc8280xp

USES=		kmod uidfix

USE_GITHUB=	yes
GH_ACCOUNT=	freebsd
GH_PROJECT=	drm-kmod-firmware
GH_TAGNAME=	20260929

MAKE_ENV=	KMODS="msmkmsfw"
MAKE_ENV+=	MSMKMODS="${FLAVOR}"

.include <bsd.port.pre.mk>

PLIST_SUB+=	${FLAVORS:N${FLAVOR}:@v@${v:tu}="\@comment "@}
PLIST_SUB+=	${FLAVOR:tu}=""

.include <bsd.port.post.mk>
