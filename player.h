//
// Created by MJVA_ on 1/1/2026.
//

#ifndef GAMEA_PLAYER_H
#define GAMEA_PLAYER_H

#include <stdint.h>

typedef struct PLAYER {
    int32_t playerId;
    char name[12];
    int32_t posX;
    int32_t posY;
    int32_t hp;
    int32_t maxHp;
    int32_t strength;
    int32_t defense;
}PLAYER;

#endif //GAMEA_PLAYER_H