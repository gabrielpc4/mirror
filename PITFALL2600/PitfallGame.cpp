#include "PitfallGame.h"
#include <iostream>
using namespace std;
void PitfallGame::handleKeyboardInput(int key, int keyState)
{
	if (key == GLUT_KEY_RIGHT)
	{
		if (player->isClimbing() == false)
		{
			if (keyState == DOWN)
			{
				player->look(RIGHT);
				player->moving(true);
			}
			else
			{
				player->moving(false);
			}
		}
		else
		{
			if (player->isAbleToClimbOut(world->stairs->hole))
			{
				player->look(RIGHT);
				player->climbOut(RIGHT);
			}			
		}
	}
	if (key == GLUT_KEY_LEFT)
	{
		if (player->isClimbing() == false)
		{
			if (keyState == DOWN)
			{

				player->look(LEFT);
				player->moving(true);
			}
			else
			{
				player->moving(false);
			}
		}
		else
		{
			if (player->isAbleToClimbOut(world->stairs->hole))
			{
				player->look(LEFT);
				player->climbOut(LEFT);
			}
		}
	}
	if (player->isFalling() == false)
	{
		if (key == GLUT_KEY_UP)
		{
			if (keyState == DOWN)
			{
				if (world->stairs != NULL)
				{
					if (checkColisionX(player, world->stairs) && player->isAbleToClimbOut(world->stairs->hole) == false)
					{
						player->setX(world->stairs->x() + 10);
						player->climbing(true);
						player->climb(UP);
					}
				}
			}
			else
			{
				player->stopClimbing();
			}
		}
		if (key == GLUT_KEY_DOWN)
		{
			if (keyState == DOWN)
			{
				if (player->isClimbing())
				{
					player->climb(DOWN);
				}
			}
			else
			{
				player->stopClimbing();
			}
		}
	}
}

void PitfallGame::handleKeyboardInput(unsigned char c)
{
	if (c == SPACE_BAR)
	{	
		if ((player->isClimbing() == false) && (player->isFalling() == false))
		{
			player->jumping(true);
		}
		if (player->isClimbing() && player->isAbleToClimbOut(world->stairs->hole))
		{
			player->climbOut(RIGHT);
		}
	}
}

PitfallGame::PitfallGame()
{
	world = new World(0);
	player = new Player(39, 140);	

}


void PitfallGame::drawAll()
{		
	world->draw();
	player->draw();

	if (world->stairs != NULL)
	{
		world->drawStairHoleCover();
	}
}

void PitfallGame::moveAll()
{	
	player->move();

	if (world->stairs != NULL)
	{	
		if (player->willFall(world->stairs->hole) && (player->isClimbing() == false) && (player->isJumping() == false))
		{		
			if (player->y() >= world->ground.y())
			{
				if (player->isJumping() == false)
				{
					player->ground_y = world->tunnelFloor.topY();
					player->falling(true);
				}
			}
		}
		if (player->isClimbing())
		{
			if (player->climbingDirection() == DOWN)
			{
				if (player->y() - CLIMBING_SPEED <= world->tunnelFloor.topY())
				{
					player->climbing(false);
					player->setSpeedY(0);
					player->setY(world->tunnelFloor.topY());
				}
			}
			else if (player->climbingDirection() == UP)
			{
				if (player->isAbleToClimbOut(world->stairs->hole))
				{				
					player->stopClimbing();					
				}
			}
		}
	}	
}

bool PitfallGame::checkColisionX(Player* player, GameObject* object)
{	
	if (player->rightX() >= object->x() && player->x() <= object->rightX())
	{		
		return true;
	}
	return false;
}


bool PitfallGame::checkColisionY(Player* player, GameObject* object)
{
	if (player->y() <= object->topY() && player->topY() >= object->y())
	{
		return true;
	}
	return false;
}



