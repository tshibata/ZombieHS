/******************************************************************************
Copyright (c) 2019, tshibata <staatsschreiber@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of tshibata nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL tshibata BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#ifndef __ZOMBIE_HS__UTIL_H__
#define __ZOMBIE_HS__UTIL_H__

#include "cocos2d.h"
USING_NS_CC;

#define FLOOR_WIDTH 17
#define FLOOR_HEIGHT 12

#define UNIT 30

#define E 0 // east
#define N 1 // north
#define W 2 // west
#define S 3 // south

#define TBD (-1) // to be determined
#define WALL 0 // not able to go through
#define HALL 1 // able to go through

extern int cos4R[4]; // cosine for R
extern int sin4R[4]; // sine for R

/*
 * direction of a relative coordinate.
 */
int direction(int dx, int dy);

/*
 * direction of the left hand, or the right when offset is negative.
 */
int turnLeft(int origin, int offset);

/*
 * check if it is
 * - not a part of wide room or thick wall
 * - not by a thin point of wall
 */
bool noMoreNoLessRule(int x, int y);

/*
 * check if there is only one passage around.
 */
bool oneWayToGoRule(int x, int y);

/*
 * make passages without bypass.
 */
void dig(int x, int y);

/*
 * try to make a bypass.
 */
bool digBypass();

/*
 * -1: to be determined
 * 0: wall
 * 1: passage
 */
extern char floorMap[FLOOR_WIDTH][FLOOR_HEIGHT];

/*
 * removable items on the ground.
 */
extern Sprite * removables[FLOOR_WIDTH][FLOOR_HEIGHT];

#endif // __ZOMBIE_HS__UTIL_H__
