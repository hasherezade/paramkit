#pragma once

#include "term_colors.h"

namespace paramkit {

    const WORD ERROR_COLOR = RED;
    const WORD WARNING_COLOR = RED;
    const WORD HILIGHTED_COLOR = WHITE;

    const WORD HEADER_COLOR = YELLOW;
    const WORD SEPARATOR_COLOR = BROWN;
    const WORD INACTIVE_COLOR = GRAY;

    const WORD PARAM_SIMILAR_NAME = MAKE_COLOR(MAGENTA, BLACK);
    const WORD PARAM_SIMILAR_DESC = MAKE_COLOR(BLACK, MAGENTA);
};
