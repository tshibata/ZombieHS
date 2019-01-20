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

#include "IndieRandom.h"
#include "Util.h"
#include "Walker.h"
#include "ZombieHSScene.h"

int arrowKeys[4] = {0, 0, 0, 0};

int secureness;

Sprite * knives[MAX_ARSENAL];
int arsenal = 5;

Animation * loadAnimation(const char * prefix, int count, float delay)
{
	char buf[100];
	auto cache = CCSpriteFrameCache::getInstance();
	auto a = Animation::create();
	for (int i = 0; i < count; i++)
	{
		snprintf(buf, sizeof(buf), "%s%d.png", prefix, i);
		a->addSpriteFrame(cache->getSpriteFrameByName(buf));
	}
	a->setDelayPerUnit(delay);
	return a;
}

Walker::Walker(int x, int y, int d, const char * prefix, int count, float delay)
{
	char buf[100];
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto cache = CCSpriteFrameCache::getInstance();
	this->d = d;
	s = Sprite::create();
	s->retain();
	s->setPosition(Vec2(origin.x + x, origin.y + y + Y_OFFSET));
	snprintf(buf, sizeof(buf), "%s.plist", prefix);
	cache->addSpriteFramesWithFile(buf);
	for (int i = 0; i < 4; i++)
	{
		snprintf(buf, sizeof(buf), "%s%d", prefix, i);
		a[i] = loadAnimation(buf, count, delay);
		a[i]->retain();
	}
	s->setSpriteFrame(a[d]->getFrames().at(0)->getSpriteFrame());
}

Walker::~Walker()
{
	s->release();
	for (int i = 0; i < 4; i++)
	{
		a[i]->release();
	}
}

Hero::Hero(int x, int y, int d, const char * prefix, int count, float delay) : Walker(x, y, d, prefix, count, delay)
{
	busy = false;
}

bool Hero::move(int d2)
{
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto pos = s->getPosition();
	int x = (int) (pos.x - origin.x) / UNIT;
	int y = (int) (pos.y - origin.y - Y_OFFSET) / UNIT;
	if (busy || cells[x + cos4R[d2]][y + sin4R[d2]] == WALL)
	{
		return false;
	}
	else
	{
		d = d2;
		busy = true;
		auto animation = a[d];
		s->runAction(Sequence::create(
			Spawn::create(
				Repeat::create(Animate::create(animation), 2),
				MoveBy::create(animation->getDuration() * 2, Vec2(UNIT * cos4R[d], UNIT * sin4R[d])),
				nullptr),
			CallFunc::create([this]() {
				auto origin = Director::getInstance()->getVisibleOrigin();
				auto pos = s->getPosition();
				int x = (int) (pos.x - origin.x) / UNIT;
				int y = (int) (pos.y - origin.y - Y_OFFSET) / UNIT;
				if (traps[x][y] != nullptr)
				{
					if (traps[x][y]())
					{
						traps[x][y] = nullptr;
					}
				}
				if (y <= 1 && secureness <= 0)
				{
					Director::getInstance()->replaceScene(ZombieHSScene::create());
					return;
				}
				busy = false;
				if (arrowKeys[turnLeft(d, 1)] != arrowKeys[turnLeft(d, -1)])
				{
					if (arrowKeys[turnLeft(d, 1)] != 0)
					{
						if (move(turnLeft(d, 1)))
						{
							return;
						}
					}
					else
					{
						if (move(turnLeft(d, -1)))
						{
							return;
						}
					}
				}
				if (arrowKeys[d] != arrowKeys[turnLeft(d, 2)])
				{
					if (arrowKeys[d] != 0)
					{
						move(d);
					}
					else
					{
						move(turnLeft(d, 2));
					}
				}
			}),
			nullptr));
		return true;
	}
}

Zombie::Zombie(int x, int y, int d, const char * prefix, int count, float delay) : Walker(x, y, d, prefix, count, delay)
{
}

void Zombie::move()
{
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto pos = s->getPosition();
	int x = (int) (pos.x - origin.x) / UNIT;
	int y = (int) (pos.y - origin.y - Y_OFFSET) / UNIT;
	if ((int) pos.x % UNIT == 0 && cells[x + cos4R[d]][y + sin4R[d]] == WALL)
	{
		d = turnLeft(d, 2);
	}
	auto animation = a[d];
	if (rand() % 2)
	{
		s->runAction(Sequence::create(
			Spawn::create(
				Animate::create(animation),
				MoveBy::create(animation->getDuration(), Vec2((UNIT / 2) * cos4R[d], (UNIT / 2) * sin4R[d])),
				nullptr),
			CallFunc::create([this]() {
				move();
			}),
			nullptr));
	}
	else
	{
		s->runAction(Sequence::create(
			DelayTime::create(animation->getDuration()),
			CallFunc::create([this]() {
				move();
			}),
			nullptr));
	}
}

Stalker::Stalker(Sprite * target, int x, int y, int d, const char * prefix, int count, float delay) : Walker(x, y, d, prefix, count, delay)
{
	this->target = target;
}

void Stalker::move()
{
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto pos = s->getPosition();
	int x = (int) (pos.x - origin.x) / UNIT;
	int y = (int) (pos.y - origin.y - Y_OFFSET) / UNIT;
	if ((int) pos.x % UNIT == 0 && (int) pos.y % UNIT == 0)
	{
		int x2 = (int) (target->getPosition().x - origin.x) / UNIT;
		int y2 = (int) (target->getPosition().y - origin.y - Y_OFFSET) / UNIT;
		d = navigate(x, y, x2, y2);
	}
	auto animation = a[d];
	if (rand() % 2)
	{
		s->runAction(Sequence::create(
			Spawn::create(
				Animate::create(animation),
				MoveBy::create(animation->getDuration(), Vec2((UNIT / 2) * cos4R[d], (UNIT / 2) * sin4R[d])),
				nullptr),
			CallFunc::create([this]() {
				move();
			}),
			nullptr));
	}
	else
	{
		s->runAction(Sequence::create(
			DelayTime::create(animation->getDuration()),
			CallFunc::create([this]() {
				move();
			}),
			nullptr));
	}
}

