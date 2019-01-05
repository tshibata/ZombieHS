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

#include "ZombieHSScene.h"
#include "SimpleAudioEngine.h"

#include <stdlib.h>

#define FLOOR_WIDTH 17
#define FLOOR_HEIGHT 12

#define UNIT 30

#define E 0 // east
#define N 1 // north
#define W 2 // west
#define S 3 // south

int clockwise(int origin, int offset);

int dir(int dx, int dy);

/*
 * -1: to be determined
 * 0: wall
 * 1: passage
 */
extern char floorMap[FLOOR_WIDTH][FLOOR_HEIGHT];

#define TBD (-1)
#define WALL 0
#define HALL 1

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


USING_NS_CC;

int cos4R[] = {+1, 0, -1, 0}; // cosine for R
int sin4R[] = {0, +1, 0, -1}; // sine for R

char floorMap[FLOOR_WIDTH][FLOOR_HEIGHT];
Sprite * removables[FLOOR_WIDTH][FLOOR_HEIGHT];

//bool arrowKeys[4] = {false, false, false, false};
int keyLeft = 0;
int keyRight = 0;
int keyUp = 0;
int keyDown = 0;

int turnLeft(int origin, int offset)
{
	return (origin + offset) & 3;
}

int direction(int dx, int dy)
{
	if (abs(dx) > abs(dy))
	{
		if (dx < 0)
		{
			return W;
		}
		else
		{
			return E;
		}
	}
	else
	{
		if (dy < 0)
		{
			return S;
		}
		else
		{
			return N;
		}
	}
}

bool noMoreNoLessRule(int x, int y)
{
	for (int dy = -1; dy < 1; dy++)
	{
		for (int dx = -1; dx < 1; dx++)
		{
			if (floorMap[x + dx][y + dy] == floorMap[x + dx + 1][y + dy + 1]
			 && floorMap[x + dx + 1][y + dy] == floorMap[x + dx][y + dy + 1])
			{
				return false;
			}
		}
	}
	return true;
}

bool oneWayToGoRule(int x, int y)
{
	int count;
	count = 0;
	for (int i = 0; i < 4; i++)
	{
		if (floorMap[x + cos4R[i]][y + sin4R[i]] == HALL)
		{
			count++;
		}
	}
	return count == 1;
}

void dig(int x, int y)
{
	if (floorMap[x][y] == TBD)
	{
		floorMap[x][y] = 1;
		if (oneWayToGoRule(x, y) && noMoreNoLessRule(x, y))
		{
			int dirs[] = {0, 1, 2, 3};
			for (int i = 4; 0 < i; i--)
			{
				int r = rand() % i;
				dig(x + cos4R[dirs[r]], y + sin4R[dirs[r]]);
				dirs[r] = dirs[i - 1];
			}
		}
		else
		{
			floorMap[x][y] = TBD;
		}
	}
}

bool digBypass()
{
	int x = rand() % (FLOOR_WIDTH - 2) + 1;
	int y = rand() % (FLOOR_HEIGHT - 2) + 1;
	if (floorMap[x][y] == TBD && noMoreNoLessRule(x, y))
	{
		floorMap[x][y] = 1;
		if (! noMoreNoLessRule(x, y))
		{
			floorMap[x][y] = TBD;
			return false;
		}
	}
	return true;
}

int stageIndex = 1;

#define MAX_ARSENAL 10
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

class Walker
{
public:
	int d;
	Sprite * s;
	Animation * a[4];
	Walker(int x, int y, int d, const char * prefix, int count, float delay);
	virtual ~Walker();
};

Walker::Walker(int x, int y, int d, const char * prefix, int count, float delay)
{
	char buf[100];
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto cache = CCSpriteFrameCache::getInstance();
	this->d = d;
	s = Sprite::create();
	s->retain();
	s->setPosition(Vec2(origin.x + x, origin.y + y));
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

class Hero : public Walker
{
public:
	bool busy;
	Hero(int x, int y, int d, const char * prefix, int count, float delay);
};

Hero::Hero(int x, int y, int d, const char * prefix, int count, float delay) : Walker(x, y, d, prefix, count, delay)
{
	busy = false;
}

class Zombie : public Walker
{
public:
	Zombie(int x, int y, int d, const char * prefix, int count, float delay);
};

Zombie::Zombie(int x, int y, int d, const char * prefix, int count, float delay) : Walker(x, y, d, prefix, count, delay)
{
}

void ramble(Zombie * mob)
{
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto pos = mob->s->getPosition();
	int x = (int) (pos.x - origin.x) / UNIT;
	int y = (int) (pos.y - origin.y) / UNIT;
	if ((int) pos.x % UNIT == 0 && floorMap[x + cos4R[mob->d]][y + sin4R[mob->d]] == WALL)
	{
		mob->d = turnLeft(mob->d, 2);
	}
	auto animation = mob->a[mob->d];
	if (rand() % 2)
	{
		mob->s->runAction(Sequence::create(
			Spawn::create(
				Animate::create(animation),
				MoveBy::create(animation->getDuration(), Vec2((UNIT / 2) * cos4R[mob->d], (UNIT / 2) * sin4R[mob->d])),
				nullptr),
			CallFunc::create([mob]() {
				ramble(mob);
			}),
			nullptr));
	}
	else
	{
		mob->s->runAction(Sequence::create(
			DelayTime::create(animation->getDuration()),
			CallFunc::create([mob]() {
				ramble(mob);
			}),
			nullptr));
	}
}

Hero * hero;

Zombie * mobs[4];

const char * mobPlists[] = {
	"danshi01",
	"danshi03",
	"joshi01",
	"joshi02",
};

void move()
{
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto pos = hero->s->getPosition();
	int x = (int) (pos.x - origin.x) / UNIT;
	int y = (int) (pos.y - origin.y) / UNIT;
	if (! hero->busy && floorMap[x + cos4R[hero->d]][y + sin4R[hero->d]] != WALL)
	{
		hero->busy = true;
		auto animation = hero->a[hero->d];
		hero->s->runAction(Sequence::create(
			Spawn::create(
				Repeat::create(Animate::create(animation), 2),
				MoveBy::create(animation->getDuration() * 2, Vec2(UNIT * cos4R[hero->d], UNIT * sin4R[hero->d])),
				nullptr),
			CallFunc::create([]() {
				auto origin = Director::getInstance()->getVisibleOrigin();
				auto pos = hero->s->getPosition();
				int x = (int) (pos.x - origin.x) / UNIT;
				int y = (int) (pos.y - origin.y) / UNIT;
				if (y <= 0)
				{
					Director::getInstance()->replaceScene(ZombieHSScene::create());
					return;
				}
				if (removables[x][y] != nullptr && arsenal < MAX_ARSENAL)
				{
					floorMap[x][y] = HALL;
					removables[x][y]->setVisible(false);
					removables[x][y] = nullptr; // leave it to ZombieHSScene
					knives[arsenal++]->setVisible(true);
				}
				hero->busy = false;
				if (keyRight - keyLeft != 0)
				{
					if (keyRight != 0)
					{
						hero->d = 0;
						move();
					}
					else
					{
						hero->d = 2;
						move();
					}
				}
				else if (keyUp - keyDown != 0)
				{
					if (keyUp != 0)
					{
						hero->d = 1;
						move();
					}
					else
					{
						hero->d = 3;
						move();
					}
				}
			}),
			nullptr));
	}
}

bool ZombieHSScene::init()
{
	if (! Scene::init())
	{
		return false;
	}

	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	char buf[10];
	snprintf(buf, sizeof(buf), "Stage%d", stageIndex++);
	auto label = Label::createWithTTF(buf, "fonts/arial.ttf", 24);
	label->setTextColor(Color4B::BLACK);
	label->setPosition(Vec2(origin.x + (7 * UNIT) + label->getContentSize().width / 2,
		origin.y + (FLOOR_HEIGHT - 2) * UNIT));
	this->addChild(label, 1000);

	for (int y = 0; y < FLOOR_HEIGHT; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			floorMap[x][y] = WALL;
			removables[x][y] = nullptr;
		}
	}
	for (int y = 1; y < FLOOR_HEIGHT - 2; y++)
	{
		for (int x = 1; x < FLOOR_WIDTH - 1; x++)
		{
			floorMap[x][y] = TBD;
		}
	}

	unsigned int seed = time(nullptr);
	printf("seed: %u\n", seed);
	srand(seed);

	floorMap[1][FLOOR_HEIGHT - 2] = HALL;
	dig(1, FLOOR_HEIGHT - 3);

	// so far there is no circular route.

	// add bypasses.
	for (int i = 0; i < 100; i++)
	{
		digBypass();
	}

	// add an exit.
	for (int x = FLOOR_WIDTH - 1;; x--)
	{
		if (x == 0)
		{
			// just in case. it never happens, i believe.
			exit(1);
		}
		if (floorMap[x][1] == HALL)
		{
			floorMap[x][0] = HALL;
			auto sprite = Sprite::create("gate.png");
			sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y));
			this->addChild(sprite, 1016);
			break;
		}
	}

	// if it is still TBD, it must be WALL.
	for (int y = 0; y < FLOOR_HEIGHT; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			if (floorMap[x][y] == TBD)
			{
				floorMap[x][y] = WALL;
			}
		}
	}

	// display the floor map.
	for (int y = 0; y < FLOOR_HEIGHT - 1; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			switch (floorMap[x][y])
			{
			case WALL:
				{
					auto sprite = Sprite::create("wall.png");
					sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT));
					this->addChild(sprite, 1000 - y * UNIT);
				}
				break;
			case HALL:
				{
					auto sprite = Sprite::create("floor.png");
					sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT));
					this->addChild(sprite, 1000 - y * UNIT);
				}
				if (oneWayToGoRule(x, y) && 0 < y && y < FLOOR_HEIGHT - 2)
				{
					removables[x][y] = Sprite::create("knife.png");
					removables[x][y]->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT));
					this->addChild(removables[x][y], 1001 - y * UNIT);
				}
				break;
			}
		}
	}
	for (int x = 0; x < FLOOR_WIDTH; x++)
	{
		auto sprite = Sprite::create("floor.png");
		sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + (FLOOR_HEIGHT - 1) * UNIT));
		this->addChild(sprite, 1000 - (FLOOR_HEIGHT - 1) * UNIT);
	}

	// display the weapons.
	for (int i = 0; i < MAX_ARSENAL; i ++)
	{
		knives[i] = Sprite::create("knife.png");
		knives[i]->setPosition(Vec2(origin.x + (i + 4) * (UNIT / 2), origin.y + (FLOOR_HEIGHT - 2) * UNIT));
		knives[i]->setVisible(false);
		this->addChild(knives[i], 1000);
	}
	for (int i = 0; i < arsenal; i ++)
	{
		knives[i]->setVisible(true);
	}

	// put the zombies.
	int density[FLOOR_WIDTH][FLOOR_HEIGHT];
	for (int y = 0; y < FLOOR_HEIGHT; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			density[x][y] = 0;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		while (true)
		{
			int x = rand() % (FLOOR_WIDTH - 2) + 1;
			int y = rand() % (FLOOR_HEIGHT - 2) + 1;
			if (floorMap[x - 1][y] != WALL && floorMap[x][y] != WALL && floorMap[x + 1][y] != WALL)
			{
			 	if ((density[x - 1][y] == 0 && density[x][y] == 0 && density[x + 1][y] == 0)
				 || rand() % 10 == 0 /* rarely crouded */)
				{
					mobs[i] = new Zombie(UNIT * x, UNIT * y, 0, mobPlists[i], 4, 0.05f);
					this->addChild(mobs[i]->s, 1016 - UNIT * y);
					ramble(mobs[i]);
					density[x][y]++;
					break;
				}
			}
		}
	}

	// put the hero.
	hero = new Hero(UNIT * 1, UNIT * (FLOOR_HEIGHT - 1), 3, "danshi04", 4, 0.075f);
	this->addChild(hero->s, 1016 - UNIT * (FLOOR_HEIGHT - 1));
	move();

	// prepare for key events.
	auto listener = EventListenerKeyboard::create();
	listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)->bool
	{
		switch (keyCode)
		{
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			keyUp = 1;
			hero->d = 1;
			move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			keyDown = 1;
			hero->d = 3;
			move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			keyLeft = 1;
			hero->d = 2;
			move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			keyRight = 1;
			hero->d = 0;
			move();
			break;
		default: // ignore others
			break;
		}
		return true;
	};
	listener->onKeyReleased = [this](cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)->bool
	{
		switch (keyCode)
		{
		case cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE:
			Director::getInstance()->end();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			keyUp = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			keyDown = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			keyLeft = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			keyRight = 0;
			break;
		default: // ignore others
			break;
		}
		return true;
	};
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	scheduleUpdate();

	return true;
}

void ZombieHSScene::update(float delta)
{
	auto v1 = hero->s->getPosition();
	hero->s->setLocalZOrder(1016 - (int) v1.y);
	for (int i = 0; i < 4; i++)
	{
		auto v2 = mobs[i]->s->getPosition();
		mobs[i]->s->setLocalZOrder(1016 - (int) v2.y);
		if (mobs[i]->s->isVisible() && abs(v1.x - v2.x) + abs(v1.y - v2.y) < UNIT / 2)
		{
			if (0 < arsenal)
			{
				knives[-- arsenal]->setVisible(false);
				mobs[i]->s->setVisible(false);
				// invisible zombie keeps walking. don't mind.
			}
			else
			{
				Director::getInstance()->end();
				// There seems to be something to care about in case of iOS...
			}
		}
	}
}
