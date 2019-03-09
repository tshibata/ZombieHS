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
#include "ZombieHSScene.h"

unsigned int level = 1;
unsigned int stageIndex = 1;
unsigned int duration = 100;

#define GATES_SPAN 3

Sprite * gates[2];

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

	secureness = 0;
	char buf[100];
	snprintf(buf, sizeof(buf), "Level%u (Stage%u)", level, stageIndex);
	auto label = Label::createWithTTF(buf, "fonts/arial.ttf", 20);
	label->setTextColor(Color4B::BLACK);
	label->setPosition(Vec2(origin.x + (7 * UNIT) + label->getContentSize().width / 2,
		origin.y + (FLOOR_HEIGHT - 4) * UNIT + Y_OFFSET));
	this->addChild(label, 1000);

	timer = Label::createWithTTF("", "fonts/arial.ttf", 20);
	timer->setTextColor(Color4B::BLACK);
	timer->setPosition(Vec2(origin.x + (15 * UNIT),
		origin.y + (FLOOR_HEIGHT - 4) * UNIT + Y_OFFSET));
	this->addChild(timer, 1000);
	tick(0);

	for (int y = 0; y < FLOOR_HEIGHT; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			cells[x][y] = WALL;
			traps[x][y] = nullptr;
		}
	}
	for (int y = 2; y < FLOOR_HEIGHT - 4; y++)
	{
		for (int x = 1; x < FLOOR_WIDTH - 2; x++)
		{
			cells[x][y] = TBD;
		}
	}

	IndieRandom rand(stageIndex);

	cells[1][FLOOR_HEIGHT - 4] = HALL;
	dig(1, FLOOR_HEIGHT - 5, rand);

	// so far there is no circular route.

	// add bypasses.
	for (int i = 0; i < 100; i++)
	{
		digBypass(rand);
	}

	// add an exit.
	for (int count = 0, x = FLOOR_WIDTH - 1;count < 2; x--)
	{
		if (x < 0)
		{
			while (count < 2)
			{
				gates[count++] = nullptr;
			}
			break;
		}
		if (cells[x][2] == HALL)
		{
			cells[x][1] = HALL;
			if (oneWayToGoRule(x, 1))
			{
				traps[x][1] = [count]()
				{
					if (secureness <= 0)
					{
						level++;
						stageIndex = (stageIndex << 1) + (1 - count);
						return true;
					}
					return false;
				};
				gates[count] = Sprite::create("gate.png");
				gates[count]->setPosition(Vec2(origin.x + x * UNIT, origin.y + UNIT + Y_OFFSET));
				this->addChild(gates[count], 1017 - UNIT);
				count++;
				x -= GATES_SPAN;
			}
			else
			{
				cells[x][1] = TBD;
			}
		}
	}

	// if it is still TBD, it must be WALL.
	for (int y = 0; y < FLOOR_HEIGHT; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			if (cells[x][y] == TBD)
			{
				cells[x][y] = WALL;
			}
		}
	}


	// put the hero.
	hero = new Hero(UNIT * 1, UNIT * (FLOOR_HEIGHT - 3), S, "danshi04", 4, 0.075f);
	this->addChild(hero->s, 1016 - UNIT * (FLOOR_HEIGHT - 2));
	hero->move(S);

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
			int x = rand.next() % (FLOOR_WIDTH - 2) + 1;
			int y = rand.next() % (FLOOR_HEIGHT - 2) + 1;
			if (cells[x - 1][y] != WALL && cells[x][y] != WALL && cells[x + 1][y] != WALL)
			{
				if ((density[x - 1][y] == 0 && density[x][y] == 0 && density[x + 1][y] == 0)
				 || rand.next() % 10 == 0 /* rarely crouded */)
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

	// path behind
	for (int x = 1; x < FLOOR_WIDTH - 1; x++)
	{
		cells[x][FLOOR_HEIGHT - 3] = HALL;
	}

	stalker = new Stalker(hero->s, UNIT * (FLOOR_WIDTH - 2), UNIT * (FLOOR_HEIGHT - 3), W, "danshi02", 4, 0.05f);
	this->addChild(stalker->s, 1016 - UNIT * (FLOOR_HEIGHT - 2));
	stalker->move();

	// items
	for (int y = 0; y < FLOOR_HEIGHT - 3; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
				if (cells[x][y] == HALL && oneWayToGoRule(x, y) && 1 < y && y < FLOOR_HEIGHT - 2)
				{
					if (rand.next() % 10 < 2)
					{
						auto sprite = Sprite::create("knife.png");
						traps[x][y] = [sprite]()
						{
							if (arsenal < MAX_ARSENAL)
							{
								sprite->setVisible(false);
								knives[arsenal++]->setVisible(true);
								return true;
							}
							return false;
						};
						sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT + Y_OFFSET));
						this->addChild(sprite, 1001 - y * UNIT);
					}
					else if (rand.next() % 8 < 3)
					{
						auto sprite = Sprite::create("key.png");
						traps[x][y] = [sprite]()
						{
							sprite->setVisible(false);
							secureness--;
							if (secureness <= 0)
							{
								for (int i = 0; i < 2; i++)
								{
									if (gates[i] != nullptr)
									{
										gates[i]->runAction(CCFadeOut::create(0.5f));
									}
								}
							}
							return true;
						};
						sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT + Y_OFFSET));
						this->addChild(sprite, 1001 - y * UNIT);
						secureness++;
					}
					else
					{
						auto sprite = Sprite::create("pill.png");
						traps[x][y] = [this, sprite]()
						{
							tick(- 10);
							sprite->setVisible(false);
							return true;
						};
						sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT + Y_OFFSET));
						this->addChild(sprite, 1001 - y * UNIT);
					}
				}
		}
	}

	// display the floor map.
	for (int y = 0; y < FLOOR_HEIGHT - 1; y++)
	{
		for (int x = 0; x < FLOOR_WIDTH; x++)
		{
			switch (cells[x][y])
			{
			case WALL:
				{
					auto sprite = Sprite::create("wall.png");
					sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT + Y_OFFSET));
					this->addChild(sprite, 1000 - y * UNIT);
				}
				break;
			case HALL:
				{
					auto sprite = Sprite::create("floor.png");
					sprite->setPosition(Vec2(origin.x + x * UNIT, origin.y + y * UNIT + Y_OFFSET));
					this->addChild(sprite, 1000 - y * UNIT);
				}
				break;
			}
		}
	}

	if (secureness <= 0)
	{
		for (int i = 0; i < 2; i++)
		{
			if (gates[i] != nullptr)
			{
				gates[i]->setVisible(false);
			}
		}
	}

	// display the weapons.
	for (int i = 0; i < MAX_ARSENAL; i ++)
	{
		knives[i] = Sprite::create("knife.png");
		knives[i]->setPosition(Vec2(origin.x + (i + 4) * (UNIT / 2), origin.y + (FLOOR_HEIGHT - 4) * UNIT + Y_OFFSET));
		knives[i]->setVisible(false);
		this->addChild(knives[i], 1000);
	}
	for (int i = 0; i < arsenal; i ++)
	{
		knives[i]->setVisible(true);
	}

	// prepare for key events.
	auto listener = EventListenerKeyboard::create();
	listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)->bool
	{
		switch (keyCode)
		{
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			arrowKeys[N] = 1;
			hero->move(N);
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			arrowKeys[S] = 1;
			hero->move(S);
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			arrowKeys[W] = 1;
			hero->move(W);
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			arrowKeys[E] = 1;
			hero->move(E);
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
	schedule(CC_SCHEDULE_SELECTOR(ZombieHSScene::tick), 1.0f);

	return true;
}

void zoom(Sprite * s)
{
	auto v1 = s->getPosition();
	s->setLocalZOrder(1016 - ((int) v1.y - Y_OFFSET));
}

void ZombieHSScene::grope(Sprite * s)
{
	auto v1 = hero->s->getPosition();
	auto v2 = s->getPosition();
	if (s->isVisible() && abs(v1.x - v2.x) + abs(v1.y - v2.y) < UNIT / 2)
	{
		if (0 < arsenal)
		{
			knives[-- arsenal]->setVisible(false);
			s->setVisible(false);
			// invisible zombie keeps walking. don't mind.
		}
		else
		{
			Director::getInstance()->end();
			// There seems to be something to care about in case of iOS...
		}
	}
}

void ZombieHSScene::update(float delta)
{
	zoom(hero->s);
	zoom(stalker->s);
	grope(stalker->s);
	for (int i = 0; i < 4; i++)
	{
		zoom(mobs[i]->s);
		grope(mobs[i]->s);
	}
}

void ZombieHSScene::tick(float delta)
{
	duration -= (int) delta;
	if (0 < duration)
	{
		char buf[10];
		snprintf(buf, sizeof(buf), "%u:%u%u", duration / 60, duration % 60 / 10, duration % 10);
		timer->setString(buf);
		if (0 <= delta)
		{
			timer->setScale(1.0f);
			timer->setOpacity(255);
			if (duration <= 10)
			{
				timer->runAction(Spawn::create(CCFadeOut::create(1.0f), CCScaleTo::create(1.0f, 2.0f), nullptr));
			}
			else
			{
				timer->runAction(CCFadeOut::create(1.0f));
			}
		}
	}
	else
	{
		Director::getInstance()->end();
		// There seems to be something to care about in case of iOS...
	}
}

void ZombieHSScene::cleanup()
{
	delete hero;
	for (int i = 0; i < 4; i++)
	{
		delete mobs[i];
	}
	delete stalker;
}
