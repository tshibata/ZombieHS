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

#include "Util.h"
#include "Walker.h"
#include "ZombieHSScene.h"

int stageIndex = 1;

Hero * hero;

Zombie * mobs[4];

const char * mobPlists[] = {
	"danshi01",
	"danshi03",
	"joshi01",
	"joshi02",
};

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
					mobs[i]->move();
					density[x][y]++;
					break;
				}
			}
		}
	}

	// put the hero.
	hero = new Hero(UNIT * 1, UNIT * (FLOOR_HEIGHT - 1), 3, "danshi04", 4, 0.075f);
	this->addChild(hero->s, 1016 - UNIT * (FLOOR_HEIGHT - 1));
	hero->move();

	// prepare for key events.
	auto listener = EventListenerKeyboard::create();
	listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)->bool
	{
		switch (keyCode)
		{
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			arrowKeys[hero->d = N] = 1;
			hero->move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			arrowKeys[hero->d = S] = 1;
			hero->move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			arrowKeys[hero->d = W] = 1;
			hero->move();
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			arrowKeys[hero->d = E] = 1;
			hero->move();
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
			arrowKeys[N] = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			arrowKeys[S] = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			arrowKeys[W] = 0;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			arrowKeys[E] = 0;
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
