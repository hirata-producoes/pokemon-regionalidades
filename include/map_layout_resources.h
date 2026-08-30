#ifndef GUARD_MAP_LAYOUT_RESOURCES_H
#define GUARD_MAP_LAYOUT_RESOURCES_H

#include "global.fieldmap.h"

#ifdef PORTABLE
const struct MapLayout *ResolvePcMapLayout(u16 mapLayoutId, const struct MapLayout *compiledLayout);
#endif

#endif // GUARD_MAP_LAYOUT_RESOURCES_H
