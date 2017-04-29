PORTNAME=	voxvision
PORTVERSION=	0.14
CATEGORIES=	graphics
MAINTAINER=	shamaz.mazum@gmail.com
COMMENT=	Voxel visualisation library

LICENSE=	BSD
LICENSE_FILE=	${WRKSRC}/LICENSE

LIB_DEPENDS=	libiniparser.so:devel/iniparser \
		libdispatch.so:devel/libdispatch

USE_SDL=	sdl2
USES=		lua cmake:outsource

USE_GITHUB=	yes
GH_ACCOUNT=	shamazmazum
GH_TAGNAME=	00adaed

.include <bsd.port.mk>
