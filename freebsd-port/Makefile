PORTNAME=	voxvision
DISTVERSION=	0.20-130
DISTVERSIONPREFIX=	v
DISTVERSIONSUFFIX=	-ge100f2f
CATEGORIES=	graphics
MAINTAINER=	shamaz.mazum@gmail.com
COMMENT=	Voxel visualisation library

LICENSE=	BSD
LICENSE_FILE=	${WRKSRC}/LICENSE

LIB_DEPENDS=	libiniparser.so:devel/iniparser \
		libdispatch.so:devel/libdispatch

USE_SDL=	sdl2
USES=		lua:53 cmake:outsource python:build

USE_GITHUB=	yes
GH_ACCOUNT=	shamazmazum
GH_TUPLE=	shamazmazum:voxvision:e100f2f:voxvision \
		shamazmazum:luasdl2:37f2163:voxvision/src/voxengine/modules/luasdl2

OPTIONS_DEFINE=	DOXYGEN NO_SSE LIGHT_MANAGER
DOXYGEN_DESC=	Install Doxygen-generated documentation
NO_SSE_DESC=	Disable SSE SIMD intrinsics (for running on old CPUs)
LIGHT_MANAGER_DESC=	Enable experimental light manager (slow)
DOXYGEN_BUILD_DEPENDS=	doxygen:devel/doxygen
NO_SSE_CMAKE_BOOL_OFF=	SSE_INTRIN
LIGHT_MANAGER_CMAKE_BOOL=	WITH_LIGHT_MANAGER
.include <bsd.port.options.mk>

do-install-DOXYGEN-on:
	${MAKE_CMD} doc -C ${BUILD_WRKSRC}
	cd ${BUILD_WRKSRC}/doc && ${COPYTREE_SHARE} . ${STAGEDIR}${DOCSDIR}

.include <bsd.port.mk>
