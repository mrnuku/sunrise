// config.h - generated by premake
// do not edit by hand, instead edit config.lua and regenerate
// it is important that you use premake recreate this file if changing xml parsers or codecs for the static builds
#define CEGUI_WITH_EXPAT 1
#define CEGUI_TINYXML_NAMESPACE CEGUITinyXML
#define CEGUI_DEFAULT_IMAGE_CODEC TGAImageCodec
#define CEGUI_HAS_DEFAULT_LOGGER 
#define CEGUI_FALAGARD_RENDERER 
#define CEGUI_TINYXML_H "ceguitinyxml/tinyxml.h"
#define CEGUI_DEFAULT_XMLPARSER ExpatParser
#define CEGUI_CODEC_TGA 1
#define CEGUI_LUA_VER 51
#if defined(_DEBUG) || defined(DEBUG)
#   define CEGUI_HAS_BUILD_SUFFIX
#   define CEGUI_BUILD_SUFFIX "_d"
#endif