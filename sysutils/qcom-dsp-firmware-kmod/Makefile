PORTNAME=	qcom-dsp-firmware-kmod
PORTVERSION=	20260930
CATEGORIES=	sysutils kld

MAINTAINER=	kanej1973@gmail.com
COMMENT=	Firmware modules for Qualcomm DSPs, started by qcom_adsp(4)
WWW=		https://github.com/JamesKane/drm-kmod-firmware

LICENSE=	QCOM
LICENSE_NAME=	Qualcomm Technologies redistributable firmware license
LICENSE_FILE=	${WRKSRC}/LICENSE.qcom
LICENSE_PERMS=	dist-mirror pkg-mirror auto-accept

ONLY_FOR_ARCHS=		aarch64
ONLY_FOR_ARCHS_REASON=	Qualcomm DSPs are only supported on aarch64

USES=		kmod uidfix

USE_GITHUB=	yes
GH_ACCOUNT=	JamesKane
GH_PROJECT=	drm-kmod-firmware
GH_TAGNAME=	79f48aa

MAKE_ENV=	KMODS="qcomfw"

PLIST_FILES=	${KMODDIR}/qcom_sc8280xp_radxa_dragon_q8b_qcadsp8280_mbn.ko

.include <bsd.port.mk>
