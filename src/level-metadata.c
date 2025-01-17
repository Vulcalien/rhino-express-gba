#include "level.h"

#include "res/levels/1.c"
#include "res/levels/2.c"
#include "res/levels/3.c"
#include "res/levels/4.c"
#include "res/levels/5.c"
#include "res/levels/6.c"
#include "res/levels/7.c"
#include "res/levels/8.c"
#include "res/levels/9.c"
#include "res/levels/10.c"
#include "res/levels/11.c"
#include "res/levels/12.c"
#include "res/levels/13.c"
#include "res/levels/14.c"
#include "res/levels/15.c"
#include "res/levels/16.c"
#include "res/levels/17.c"

const struct level_Metadata level_metadata[LEVEL_COUNT] = {
    // Level 1
    {
        .tile_data = level_1,
        .size = { 7, 3 },
        .spawn = { 1, 1 },

        .mailboxes = {
            { 4, 1 }
        },
        .houses = {
            { 5, 1 }
        },

        .tutorial_text = 1
    },

    // Level 2
    {
        .tile_data = level_2,
        .size = { 7, 8 },
        .spawn = { 5, 3 },

        .mailboxes = {
            { 3, 2 },
            { 3, 5 }
        },
        .grass = {
            { 24, 52  },
            { 28, 64  },
            { 20, 72  },
            { 72, 96  },
            { 88, 104 }
        },
        .houses = {
            { 1, 6 }
        }
    },

    // Level 3
    {
        .tile_data = level_3,
        .size = { 6, 8 },
        .spawn = { 1, 4 },

        .mailboxes = {
            { 2, 2 },
            { 3, 4 }
        },
        .grass = {
            { 72, 20  },
            { 72, 32  },
            { 52, 40  },
            { 20, 48  },
            { 20, 100 },
            { 76, 104 }
        },
        .houses = {
            { 1, 1 },
            { 4, 3 }
        }
    },

    // Level 4
    {
        .tile_data = level_4,
        .size = { 7, 5 },
        .spawn = { 5, 1 },

        .mailboxes = {
            { 2, 2 }
        },
        .grass = {
            { 80, 36 },
            { 56, 44 },
            { 88, 52 }
        },
        .houses = {
            { 1, 1, true }
        }
    },

    // Level 5
    {
        .tile_data = level_5,
        .size = { 7, 8 },
        .spawn = { 5, 4 },

        .mailboxes = {
            { 3, 2 },
            { 2, 4 }
        },
        .grass = {
            { 40, 24  },
            { 88, 84  },
            { 20, 88  },
            { 92, 100 }
        },
        .houses = {
            { 1, 1 },
            { 4, 2 },
            { 4, 6 }
        }
    },

    // Level 6
    {
        .tile_data = level_6,
        .size = { 10, 7 },
        .spawn = { 4, 3 },

        .mailboxes = {
            { 5, 2 },
            { 7, 3 },
            { 3, 4 }
        },
        .grass = {
            { 56, 48 },
            { 44, 84 },
            { 92, 88 }
        },
        .houses = {
            { 6, 1 },
            { 8, 3 },
            { 1, 4 }
        }
    },

    // Level 7
    {
        .tile_data = level_7,
        .size = { 6, 3 },
        .spawn = { 1, 1 },
        .obstacles = { 1, 0, 0 },

        .mailboxes = {
            { 3, 1 }
        },

        .tutorial_text = 2,
        .delay_editing_tutorial = true
    },

    // Level 8
    {
        .tile_data = level_8,
        .size = { 6, 6 },
        .spawn = { 2, 4 },
        .obstacles = { 2, 0, 0 },

        .mailboxes = {
            { 2, 2 }
        },
        .grass = {
            { 72, 24 },
            { 24, 56 }
        },
        .houses = {
            { 1, 1 }
        }
    },

    // Level 9
    {
        .tile_data = level_9,
        .size = { 8, 7 },
        .spawn = { 2, 5 },
        .obstacles = { 0, 2, 0 },

        .mailboxes = {
            { 2, 2 },
            { 5, 5 }
        },
        .grass = {
            { 20,  20 },
            { 100, 68 },
            { 56,  72 }
        },
        .houses = {
            { 1, 3 },
            { 1, 4, true }
        }
    },

    // Level 10
    {
        .tile_data = level_10,
        .size = { 11, 6 },
        .spawn = { 5, 4 },
        .obstacles = { 0, 1, 0 },

        .mailboxes = {
            { 2, 2 },
            { 8, 3 }
        },
        .grass = {
            { 52,  20 },
            { 136, 20 },
            { 108, 36 },
            { 72,  64 },
            { 120, 72 }
        },
        .houses = {
            { 1, 1 },
            { 9, 2, true }
        }
    },

    // Level 11
    {
        .tile_data = level_11,
        .size = { 7, 7 },
        .spawn = { 2, 3 },
        .obstacles = { 0, 3, 0 },

        .mailboxes = {
            { 3, 2 },
            { 4, 3 },
            { 2, 4 }
        },
        .grass = {
            { 88, 36 },
            { 72, 68 },
            { 40, 84 },
            { 88, 84 }
        },
        .houses = {
            { 3, 1 },
            { 1, 2 },
            { 1, 5 }
        }
    },

    // Level 12
    {
        .tile_data = level_12,
        .size = { 10, 4 },
        .spawn = { 4, 2 },

        .mailboxes = {
            { 2, 2 }
        },
        .houses = {
            { 1, 1 }
        },

        .tutorial_text = 3,
        .tutorial_bubbles = true
    },

    // Level 13
    {
        .tile_data = level_13,
        .size = { 6, 11 },
        .spawn = { 1, 6 },

        .mailboxes = {
            { 3, 2 }
        },
        .grass = {
            { 36, 40 },
            { 44, 52 }
        },
        .houses = {
            { 2, 1 }
        },

        .tutorial_bubbles = true
    },

    // Level 14
    {
        .tile_data = level_14,
        .size = { 8, 7 },
        .spawn = { 1, 5 },
        .obstacles = { 1, 1, 0 },

        .mailboxes = {
            { 5, 2 }
        },
        .grass = {
            { 104, 20 },
            { 108, 40 },
            { 104,  64 }
        },
        .houses = {
            { 4, 1 }
        },

        .tutorial_text = 4
    },

    // Level 15
    {
        .tile_data = level_15,
        .size = { 9, 5 },
        .spawn = { 4, 3 },
        .obstacles = { 1, 0, 1 },

        .mailboxes = {
            { 2, 3 },
            { 7, 2 }
        },
        .grass = {
            { 44, 20 },
            { 80, 20 },
            { 64, 32 },
            { 88, 36 }
        },
        .houses = {
            { 1, 1 },
            { 1, 2 }
        }
    },

    // Level 16
    {
        .tile_data = level_16,
        .size = { 9, 9 },
        .spawn = { 4, 4 },
        .obstacles = { 2, 1, 1 },

        .mailboxes = {
            { 3, 2 },
            { 2, 3 },
            { 3, 6 },
            { 6, 6 }
        },
        .grass = {
            { 72,  28  },
            { 104, 68  },
            { 76,  84  },
            { 24,  88  },
            { 36,  112 },
            { 88,  116 },
            { 120, 120 }
        },
        .houses = {
            { 1, 1 },
            { 2, 1 },
            { 1, 2 },
            { 1, 3 }
        }
    },

    // Level 17
    {
        .tile_data = level_17,
        .size = { 11, 7 },
        .spawn = { 1, 1 },
        .obstacles = { 3, 1, 2 },

        .mailboxes = {
            { 5, 1 },
            { 7, 2 },
            { 3, 3 },
            { 8, 3 },
            { 4, 4 }
        },
        .grass = {
            { 152, 20 },
            { 20,  36 },
            { 88,  36 },
            { 104, 40 },
            { 40,  72 },
            { 56,  84 },
            { 120, 84 }
        },
        .houses = {
            { 1, 4 },
            { 1, 5 }
        }
    }
};
