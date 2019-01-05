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

int cos4R[] = {+1, 0, -1, 0};
int sin4R[] = {0, +1, 0, -1};

char cells[FLOOR_WIDTH][FLOOR_HEIGHT];

std::function<bool()> traps[FLOOR_WIDTH][FLOOR_HEIGHT] = {nullptr};

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
			if (cells[x + dx][y + dy] == cells[x + dx + 1][y + dy + 1]
			 && cells[x + dx + 1][y + dy] == cells[x + dx][y + dy + 1])
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
		if (cells[x + cos4R[i]][y + sin4R[i]] == HALL)
		{
			count++;
		}
	}
	return count == 1;
}

void dig(int x, int y)
{
	if (cells[x][y] == TBD)
	{
		cells[x][y] = 1;
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
			cells[x][y] = TBD;
		}
	}
}

bool digBypass()
{
	int x = rand() % (FLOOR_WIDTH - 2) + 1;
	int y = rand() % (FLOOR_HEIGHT - 2) + 1;
	if (cells[x][y] == TBD && noMoreNoLessRule(x, y))
	{
		cells[x][y] = 1;
		if (! noMoreNoLessRule(x, y))
		{
			cells[x][y] = TBD;
			return false;
		}
	}
	return true;
}

