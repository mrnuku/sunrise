#ifndef SYSTEM_LIB_H
#define SYSTEM_LIB_H

#define dSINGLE
#include "ode/ode.h"

#define CEGUI_STATIC
#include "CEGUI.h"
#include "CEGUIRenderer.h"
#include "CEGUISize.h"
#include "CEGUIVector.h"
#include "CEGUILogger.h"
#include "WindowRendererSets\Falagard\FalModule.h"

#ifdef __linux__
#include <GL/glx.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"

#include "al.h"
#include "alc.h"
#include "al-prot.h"
#include "efx.h"
#include "efx-creative.h"

namespace ZLib {

#include "zlib.h"
#include "ioapi.h"

#ifdef HAVE_BZIP2
#include "bzlib.h"
#endif

#include "unzip.h"
}

namespace Png {

#include "png/pnglibconf.h"
#include "png/pngconf.h"
#include "png/png.h"
}

namespace Expat {

#define XML_BUILDING_EXPAT 1
#define XMLCALL
#include "expat\expat.h"
}

namespace faac {

#include "faac.h"
}

namespace Lua {

#include "lua52/lua.h"
#include "lua52/lstate.h"
#include "lua52/lualib.h"
#include "lua52/lauxlib.h"
}

#endif
