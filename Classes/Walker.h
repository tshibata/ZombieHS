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

#ifndef __ZOMBIE_HS__WALKER_H__
#define __ZOMBIE_HS__WALKER_H__

#include "cocos2d.h"
USING_NS_CC;

extern int arrowKeys[];

extern int secureness;

#define MAX_ARSENAL 10
extern Sprite * knives[];
extern int arsenal;

class Walker
{
public:
	int d;
	Sprite * s;
	Animation * a[4];
	Walker(int x, int y, int d, const char * prefix, int count, float delay);
	virtual ~Walker();
};

class Hero : public Walker
{
public:
	bool busy;
	Hero(int x, int y, int d, const char * prefix, int count, float delay);
	bool move(int d);
};

class Zombie : public Walker
{
public:
	Zombie(int x, int y, int d, const char * prefix, int count, float delay);
	void move();
};

class Stalker : public Walker
{
private:
	Sprite * target;
public:
	Stalker(Sprite * target, int x, int y, int d, const char * prefix, int count, float delay);
	void move();
};

#endif // __ZOMBIE_HS__WALKER_H__
