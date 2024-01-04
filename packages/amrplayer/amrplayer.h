/*
 * Copyright (c) 2006-2030, Hnhinker Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2023-03-29     ccy    first implementation
 */

#ifndef PACKAGES_AMRPLAYER_AMRPLAYER_H_
#define PACKAGES_AMRPLAYER_AMRPLAYER_H_

/**
 * amr player status
 */
enum PLAYER_STATE
{
    PLAYER_STATE_STOPED  = 0,
    PLAYER_STATE_PLAYING = 1,
    PLAYER_STATE_PAUSED  = 2,
};

/**
 * @brief             Play amr music
 *
 * @param uri         the pointer for file path
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int amrplayer_play(char *uri);

/**
 * @brief             Stop music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int amrplayer_stop(void);

/**
 * @brief             Pause music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int amrplayer_pause(void);

/**
 * @brief             Resume music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int amrplayer_resume(void);

/**
 * @brief             Set volume
 *
 * @param volume      volume value(0 ~ 99)
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int amrplayer_volume_set(int volume);

/**
 * @brief             Get volume
 *
 * @return            volume value(0 ~ 99)
 */
int amrplayer_volume_get(void);

/**
 * @brief             Get amr player state
 *
 * @return
 *      - PLAYER_STATE_STOPED   stoped status
 *      - PLAYER_STATE_PLAYING  playing status
 *      - PLAYER_STATE_PAUSED   paused
 */
int amrplayer_state_get(void);

/**
 * @brief             Get the uri that is currently playing
 *
 * @return            uri that is currently playing
 */
char *amrplayer_uri_get(void);



#endif /* PACKAGES_AMRPLAYER_AMRPLAYER_H_ */
