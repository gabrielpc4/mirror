#include "PitfallGame.h"

PitfallGame::PitfallGame()
	: enemiesFile("scenario_files/ScenarioEnemies.txt")
{
	scenarioNumber			= 0;	
	frames					= 0;
	score					= 2000;
	DEBUG_MODE				= false;
	GOD_MODE				= false;	
	paused					= false;
	showCollisionRectangles = false;
	playingHitSound			= false;
	playingTreasureSound	= false;
	gameStarted				= false;
	world					= new World();
	player					= new Player(39, 140);
	enemiesGenerator		= new EnemiesGenerator(positiveScenarios, negativeScenarios, scenarioNumber, world);
	scorpion				= NULL;
	snake					= NULL;	

	if (RANDOM_LEVELS)
	{
		srand(time(NULL));
	}

	if (enemiesFile.is_open())
	{		
		ScenarioEnemies currentScenario;
		for (int i = 0; i <= 15; i++)
		{
			enemiesFile.seekToScenario(i);
			currentScenario.nScorpions	= enemiesFile.nextCharAsInt();
			currentScenario.nLogs		= enemiesFile.nextCharAsInt();
			currentScenario.movingLogs	= enemiesFile.nextCharAsBool();
			currentScenario.nCrocodiles = enemiesFile.nextCharAsInt();
			currentScenario.nSnakes		= enemiesFile.nextCharAsInt();

			positiveScenarios.push_back(currentScenario);

		}

		for (int i = 0; i >= -6; i--)
		{
			enemiesFile.seekToScenario(i);
			currentScenario.nScorpions	= enemiesFile.nextCharAsInt();
			currentScenario.nLogs		= enemiesFile.nextCharAsInt();
			currentScenario.movingLogs	= enemiesFile.nextCharAsBool();
			currentScenario.nCrocodiles = enemiesFile.nextCharAsInt();
			currentScenario.nSnakes		= enemiesFile.nextCharAsInt();

			negativeScenarios.push_back(currentScenario);
		}
	}
	else
	{
		srand(time(NULL));
	}

	world->buildScenario(scenarioNumber);	
	spawnEnemies();
	crocodiles.push_back(Crocodile(187, 136));
	crocodiles.push_back(Crocodile(245, 136));
	crocodiles.push_back(Crocodile(303, 136));		
}


void PitfallGame::spawnEnemies()
{
	if (scenarioNumber > (int)positiveScenarios.size() - 1 || abs(scenarioNumber) >(int)negativeScenarios.size() - 1)
	{
		enemiesGenerator->generateNewEnemies();
	}
	
		
	switch (thisScenario().nLogs)
	{
		case (1) :
		{
			logs.push_back(Log(LOG_POSITION_2, thisScenario().movingLogs));
		}break;
		case(2) :
		{
			logs.push_back(Log(LOG_POSITION_2, thisScenario().movingLogs));
			logs.push_back(Log(LOG_POSITION_3, thisScenario().movingLogs));
		}break;
		case(3):
		{
			logs.push_back(Log(LOG_POSITION_1, thisScenario().movingLogs));
			logs.push_back(Log(LOG_POSITION_2, thisScenario().movingLogs));
			logs.push_back(Log(LOG_POSITION_3, thisScenario().movingLogs));
		}break;
		default:
		{

		}break;
	}

	if (thisScenario().nScorpions != 0)
	{
		scorpion = new Scorpion();
	}


	if (thisScenario().nSnakes != 0)
	{
		snake = new Snake();
	}

}

void PitfallGame::run()
{
	if (gameStarted)
	{
		if (player->isDead() && player->isFalling() == false)
		{
			paused = true;
		}

		if (isPaused() == false)
		{
			moveAll();
			checkBoundaries();
			physics();
			checkCollisionsWithEnemies();

			if (world->hasAVine())
			{
				if ((player->isSwinging() == false) && player->isJumping() && canGrabVine(player))
				{
					player->holdVine(true);
					PlaySound(NULL, NULL, SND_ASYNC);
					PlaySound(TEXT("sounds/PitfallTheme.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
				if (player->isHoldingVine())
				{
					player->swing(world->vine);
				}
			}

			if (player->isTakingHit() && player->isDead() == false && score > 0)
			{
				score--;
			}
		}

		if (player->isDead() || player->isDead() && player->isFalling() == false)
		{
			player->incrementFramesDead();

			if (player->framesDead() > RESPAWN_FRAMES && player->livesLeft() > 0)
			{
				player->respawn();
				world->blackHole->enableSizeChanging();
				world->water->enableSizeChanging();
				paused = false;
			}
		}

		if (world->hasATreasure())
		{
			if (areColliding(player, *world->treasure, BOX_DETECTION))
			{
				score += 4000;
				PlaySound(NULL, NULL, NULL);
				PlaySound(TEXT("sounds/TreasureCollected.wav"), NULL, SND_ASYNC | SND_FILENAME);
				playingTreasureSound = true;
				world->deleteTreasure();
			}
		}

		if (playingTreasureSound)
		{
			if (frames > 30)
			{
				frames = 0;
				playingTreasureSound = false;
			}
			else
			{
				frames++;
			}
		}

		playSounds();
	}	
}

void PitfallGame::playSounds()
{
	if (player->isDead())
	{
		if (player->framesDead() == 1)
		{
			PlaySound(NULL, NULL, SND_ASYNC);
			PlaySound(TEXT("sounds/death.wav"), NULL, SND_ASYNC | SND_FILENAME);
		}
	}
	else if (player->isFalling() == false && player->isJumping() == false)
	{
		if (player->isTakingHit())
		{
			if (playingHitSound == false)
			{
				PlaySound(TEXT("sounds/playerHit.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
				playingHitSound = true;
			}
		}
		else
		{
			if (playingHitSound)
			{
				PlaySound(NULL, NULL, SND_ASYNC);
				playingHitSound = false;
			}

			if (player->isWalking() && player->getAnimationFrame() == 1)
			{
				if (playingTreasureSound == false)
				{
					PlaySound(TEXT("sounds/footstep.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
			}
		}
	}
}

void PitfallGame::drawAll()
{
	if (gameStarted == false)
	{
		introScreen.show();
	}
	else
	{
		world->draw(scenarioNumber);
		player->draw();
		world->drawOverlayers();

		if (thisScenario().nSnakes != 0)
		{
			snake->draw();
		}

		if (thisScenario().nScorpions != 0)
		{
			if (showCollisionRectangles == false)
			{
				scorpion->draw();
			}
		}

		for (unsigned i = 0; i < logs.size(); i++)
		{
			logs.at(i).draw();
		}

		if (thisScenario().nCrocodiles != 0)
		{
			for (unsigned i = 0; i < crocodiles.size(); i++)
			{
				crocodiles.at(i).draw();
			}
		}
		else
		{
			for (unsigned i = 0; i < crocodiles.size(); i++)
			{
				crocodiles.at(i).animate();
			}
		}

		if (showCollisionRectangles)
		{
			if (player->isUndeground() && thisScenario().nScorpions != 0
				|| world->hasABonfire() && areClose(player, (Sprite*)world->bonfire)
				|| thisScenario().nSnakes != 0 && areClose(player, (Sprite*)snake))
			{
				player->DRAW_ON_DEBUG_MODE();
			}


			if (thisScenario().nScorpions != 0)
			{
				scorpion->DRAW_ON_DEBUG_MODE();
			}

			if (thisScenario().nSnakes != 0)
			{
				snake->DRAW_ON_DEBUG_MODE();
			}

			drawCollisionRectangles();

			if (world->hasAVine())
			{
				world->vine->drawCircleTrack();
			}

			if (world->hasABonfire())
			{
				world->bonfire->DRAW_ON_DEBUG_MODE();
			}
		}

		showHUD();

		if ((player->isDead() == false) || (player->isDead()) && player->isFalling()) // Makes the game freeze, when the player dies
		{
			glutSwapBuffers();
		}
	}	
}

void PitfallGame::showHUD()
{
	string label = "Score: ";
	printText(label + std::to_string(score), Point(10, 360));

	label = "Lives: ";
	printText(label + std::to_string(player->livesLeft()), Point(10, 340));
	
	if (player->isDead() && player->livesLeft() == 0 && player->framesDead() == RESPAWN_FRAMES)
	{ 
		printText("Press Space to Restart the Game", Point(10, 5));			
		glutSwapBuffers();
	}
	else if (DEBUG_MODE)
	{
		if (GOD_MODE)
		{
			printText("God Mode ON", Point(10, 5));
		}
		else
		{
			printText("Debug Mode ON", Point(10, 5));
		}

		label = "Scenario: ";
		printText(label + std::to_string(scenarioNumber), Point(400, 5));				
	}
}

void PitfallGame::moveAll()
{
	player->move();

	// Makes the player be able to climb
	if (player->isClimbing())
	{
		if (player->climbingDirection() == DOWN)
		{
			// If the player has climbed all the way down the ladder
			if (player->y() - CLIMBING_SPEED < world->tunnelFloor.topY())
			{
				// Prevent him from going below the floor
				player->climbing(false);
				player->setSpeedY(0);
				player->setY(world->tunnelFloor.topY());
			}
		}
		else if (player->climbingDirection() == UP)
		{
			if (isAbleToClimbOut(player))
			{
				player->stopClimbing();
			}
		}
	}

	for (unsigned i = 0; i < logs.size(); i++)
	{
		if (logs.at(i).isRolling())
		{
			logs.at(i).roll();
		}
	}

	world->vine->swing();

	if (scorpion != NULL)
	{
		scorpion->move(player->x());
	}
}

void PitfallGame::physics()
{	
	if (world->ladder != NULL)
	{
		// Makes the player fall
		if (willFall(player, world->ladder->exit) && (player->isClimbing() == false))
		{
			if (player->isJumping() == false)
			{	
				player->setFloor(world->tunnelFloor.topY());
				player->falling(true);				
				PlaySound(NULL, NULL, SND_ASYNC);
				PlaySound(TEXT("sounds/falling.wav"), NULL, SND_ASYNC | SND_FILENAME);
			}
		}		
	}
	// Collision with the brick wall
	if (world->brickWall != NULL)
	{	
		if (player->isUndeground())
		{
			if (world->brickWall->x() != 0) // Prevents the collision check bug when the brick wall's sprite is being 		
			{								// created and still is at the origin and was not moved to the right position yet
				// If the player is colliding with the brick wall
				if (areColliding(player, *world->brickWall, BOX_DETECTION))
				{					
					// if the player is at the left side of the brick wall
					if (relativePosition(player, world->brickWall) == RIGHT)
					{
						// Moves the player away from the brick wall
						player->setX(world->brickWall->x() - PLAYER_ANIMATION_0_WIDTH);
					}
					else // if the player is at the right side of the brick wall
					{
						// Move the player away from the brick wall
						player->setX(world->brickWall->rightX());				
					}
				}
			}
		}
	}

	// Makes the player fall if is in contact with a ground  hole		
	for (unsigned i = 0; i < world->tunnelHole.size(); i++)
	{
		if (willFall(player, &(world->tunnelHole.at(i))))
		{
			if (player->isJumping() == false)
			{											
				player->setFloor(world->tunnelFloor.topY());
				player->falling(true);
				PlaySound(TEXT("sounds/falling.wav"), NULL, SND_ASYNC | SND_FILENAME);
			}
		}
	}	
	// Falling in the Black Hole or in the Water
	
	if (world->hasABlackHole())
	{			
		if (willFall(player, world->blackHole))
		{			
			if (player->isJumping() == false)
			{
				if (GOD_MODE == false)
				{
					world->blackHole->disableSizeChanging();					
					player->setFloor(world->tunnelTop.y());
					player->falling(true);
					player->die();
					
				}
			}
		}
	}

	if (world->hasWater())
	{
		if (willFall(player, world->water) && player->isStandingOnCrocodile() == false)
		{			
			if (player->isJumping() == false)
			{
				if (GOD_MODE == false)
				{					
					world->water->disableSizeChanging();
					player->setFloor(world->tunnelTop.y());
					player->falling(true);
					player->die();
				}
			}
		}
	}
	

}

bool PitfallGame::isAbleToClimbOut(Player* player)
{
	return (player->topY() > (world->ladder->exit->y() + 10));
}

void PitfallGame::centerOnStair(Player* player)
{
	player->setX(world->ladder->x() + 5);
}

bool  PitfallGame::willFall(Player* player, Sprite* hole)
{
	if (hole != NULL)
	{
		if (player->isUndeground() == false)  // Can only fall if it's on the top floor
		{
			if (player->isLooking() == RIGHT)
			{
				if (player->x() >= hole->x() && player->rightX() <= hole->rightX())
				{
					return true;
				}
			}
			else
			{
				if (player->rightX()  < hole->rightX()
					&& player->x() >= hole->x())
				{
					return true;
				}
			}
			return false;
		}
	}	
	return false;
}

void PitfallGame::checkBoundaries()
{	
	for (unsigned i = 0; i < logs.size(); i++)
	{		
		if (logs.at(i).rightX() <= 0)
		{
			logs.at(i).setX(WORLD_WINDOW_WIDTH);
		}
	}

	if (isOutOfBoundaries(player))
	{
		if (player->rightX() >= WORLD_WINDOW_WIDTH)
		{							
			nextScenario();
			player->setX(22);
		}
		else
		{
			if (player->x() < 0)
			{				
				previousScenario();
				player->setX(WORLD_WINDOW_WIDTH - 32);
			}
		}
	}
}

void PitfallGame::nextScenario()
{
	world->deleteWorld();
	deleteEnemies();
	scenarioNumber++;
	world->buildScenario(scenarioNumber);
	spawnEnemies();
}
void PitfallGame::previousScenario()
{
	world->deleteWorld();
	deleteEnemies();
	scenarioNumber--;
	world->buildScenario(scenarioNumber);
	spawnEnemies();
}

int PitfallGame::relativePosition(Player* player, Sprite* object)
{
	int object_center_x = (object->x() + (object->width() / 2.0));
	if (player->x() < object_center_x)
	{
		return RIGHT;
	}
	else if (player->rightX() > object_center_x)
	{
		return LEFT;
	}
	return NONE;
}

bool PitfallGame::canGrabVine(Player* player)
{
	return isInside(world->vine->end(), Rect(player->x(), player->y(), player->width(), player->height()));
}


void PitfallGame::checkCollisionsWithEnemies()
{
	// Collision with the Enemy log
	player->takeHit(false);
	for (unsigned i = 0; i < logs.size(); i++)
	{
		if (areColliding(player, (Sprite)logs.at(i), BOX_DETECTION) && player->isSwinging() == false)
		{
			player->takeHit(true);	
		}
	}

	player->standOnCrocodile(false);

	if (thisScenario().nCrocodiles != 0)
	{
		for (unsigned i = 0; i < crocodiles.size(); i++)
		{
			if (areCollidingX(player, &crocodiles[i]))
			{
				player->standOnCrocodile(true);

				if (player->y() > crocodiles[i].topY())	// Prevents the bug when the crocodile closes it's mouth when the player is between the mouth
				{
					player->setFloor(crocodiles[i].topY());
				}
				
			}
		}
	}
	

	if (thisScenario().nScorpions != 0)
	{
		// If the player is close to the scorpion
		if (areClose(player, scorpion) )
		{			
			// Only then to the pixel by pixel collision detection, because it costs a lot of processing
			if (areColliding(player, (Sprite)(*scorpion), PIXEL_BY_PIXEL_DETECTION))
			{ 
				if (GOD_MODE == false)
				{
					player->die();
				}				
			}
		}					
	}

	if (thisScenario().nSnakes != 0)
	{
		// If the player is close to the scorpion
		if (areClose(player, snake))
		{
			// Only then to the pixel by pixel collision detection, because it costs a lot of processing
			if (areColliding(player, (Sprite)(*snake), PIXEL_BY_PIXEL_DETECTION))
			{
				if (GOD_MODE == false)
				{
					player->die();
				}
			}
		}
	}

	if (world->hasABonfire())
	{
		// If the player is close to the scorpion
		if (areClose(player, world->bonfire))
		{
			// Only then to the pixel by pixel collision detection, because it costs a lot of processing
			if (areColliding(player, (Sprite)(*world->bonfire), PIXEL_BY_PIXEL_DETECTION))
			{
				if (GOD_MODE == false)
				{
					player->die();
				}
			}
		}
	}
}

bool PitfallGame::areCollidingX(Player* player, Crocodile* crocodile)
{
	if (player->y() >= 142)
	{
		if (player->rightX() > crocodile->x() && player->x() < crocodile->rightX())
		{
			return true;
		}
		return false;
	}	
	return false;
}

bool PitfallGame::areColliding(Player* player, Sprite& object, int detectionType)
{
	if (detectionType == BOX_DETECTION)
	{
		if (player->y() <= object.topY() && player->topY() >= object.y())
		{
			if (player->rightX() > object.x() && player->x() < object.rightX())
			{
				return true;
			}
			return false;				
		}
		return false;
	}
	else
	{		
		for (vector<Polygons>::iterator playerPolygons = player->begin(); playerPolygons != player->end(); playerPolygons++)
		{		
			for (vector<Rect>::iterator playerRect = playerPolygons->begin(); playerRect != playerPolygons->end(); playerRect++)
			{
				for (vector<Polygons>::iterator objectPolygons = object.begin(); objectPolygons != object.end(); objectPolygons++)
				{
					for (vector<Rect>::iterator objectRect = objectPolygons->begin(); objectRect != objectPolygons->end(); objectRect++)
					{				
						if (isInside(*playerRect, *objectRect))
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}		
	return false;
}

bool PitfallGame::isOutOfBoundaries(Player* player)
{
	if (player->x() <= 0 || player->rightX() >= WORLD_WINDOW_WIDTH)
	{
		return true;
	}
	return false;
}

void  PitfallGame::printText(string text, Point p)
{
	glColor3f(1.0, 1.0, 1.0);
	char *c;
	float scale = 0.1;
	float orientation = 0;
	glPushMatrix();
	glTranslatef(p.x(), p.y(), 0); 
	glScalef(scale, scale, scale);
	glRotatef(orientation, 0, 0, 1);	
	for (c = (char*)text.c_str(); *c; c++)
	{
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
	}
	glPopMatrix();
}

void PitfallGame::handleKeyboardInput(int key, int keyState)
{
	if (gameStarted)
	{
		if (key == GLUT_KEY_RIGHT)
		{
			if (player->isClimbing() == false) // Prevents the player sprite from changing the animation if the right key is pressed
			{
				if (keyState == DOWN)
				{
					if (player->isJumping() == false
						&& player->isFalling() == false)	// Prevents the player sprite from changing direction while jumping
					{
						player->look(RIGHT);
						player->walking(true);
					}
				}
				else // If the user releases the key
				{
					player->walking(false);
				}
			}
			else // If the player is climbing
			{
				if (isAbleToClimbOut(player))
				{
					player->look(RIGHT);
					player->climbOut(RIGHT);
					PlaySound(NULL, NULL, SND_ASYNC);
					PlaySound(TEXT("sounds/jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
			}
		}
		if (key == GLUT_KEY_LEFT)
		{
			if (player->isClimbing() == false) // Prevents the player sprite from changing the animation if the left key is pressed
			{
				if (keyState == DOWN)
				{
					if (player->isJumping() == false && player->isFalling() == false) // Prevents the player sprite from changing direction while jumping
					{
						player->look(LEFT);
						player->walking(true);
					}
				}

				else // If the user releases the key
				{
					player->walking(false);
				}
			}
			else // If the player is climbing
			{
				if (isAbleToClimbOut(player))
				{
					player->look(LEFT);
					player->climbOut(LEFT);
					PlaySound(NULL, NULL, SND_ASYNC);
					PlaySound(TEXT("sounds/jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
			}
		}
		if (player->isFalling() == false)
		{
			if (key == GLUT_KEY_UP)
			{
				if (keyState == DOWN)
				{
					if (player->isSwinging())
					{
						player->holdVine(false);
					}
					else // If the player is not swinging
					{
						if (world->ladder != NULL)
						{
							// If the player is in contact with the ladder and not about to climb out
							if (areColliding(player, *world->ladder, BOX_DETECTION) && (isAbleToClimbOut(player) == false))
							{
								centerOnStair(player);
								player->walking(false);
								player->climbing(true);
								player->setAnimationFrame(6);
								player->climb(UP);
							}
						}

						// Prevents the user from jumping while climbing or falling
						if ((player->isClimbing() == false) && (player->isFalling() == false))
						{
							if (player->isJumping() == false && player->isDead() == false)
							{
								PlaySound(NULL, NULL, SND_ASYNC);
								PlaySound(TEXT("sounds/jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
							}
							player->jumping(true);
						}
					}
				}
				else // If the user releases the UP arrow key
				{
					if (player->isClimbing())
					{
						player->stopClimbing();
					}
				}
			}
			if (key == GLUT_KEY_DOWN)
			{
				if (keyState == DOWN)
				{
					if (player->isSwinging())
					{
						player->holdVine(false);
					}
					else // If the player is not swinging
					{
						if (player->isClimbing())
						{
							player->climb(DOWN);
						}
					}
				}
				else // If the user releases the DOWN arrow key
				{
					if (player->isClimbing())
					{
						player->stopClimbing();
					}
				}
			}
		}
		if (key == GLUT_KEY_PAGE_UP)
		{
			if (keyState == DOWN)
			{
				if (DEBUG_MODE)
				{
					nextScenario();
				}
			}
		}
		else if (key == GLUT_KEY_PAGE_DOWN)
		{
			if (keyState == DOWN)
			{
				if (DEBUG_MODE)
				{
					previousScenario();
				}
			}
		}
	}
}

void PitfallGame::handleKeyboardInput(unsigned char c)
{
	if (c == SPACE_BAR)
	{
		if (gameStarted)
		{	
			// Makes the player release the vine when the space-bar is pressed
			if (player->isSwinging())
			{
				player->holdVine(false);
			}
			else if (player->isDead() && player->livesLeft() == 0 && player->framesDead() >= RESPAWN_FRAMES)
			{
				PitfallGame::reset();
			}
			else
			{
				// Prevents the user from jumping while climbing or falling
				if ((player->isClimbing() == false) && (player->isFalling() == false))
				{
					if (player->isJumping() == false && player->isDead() == false)
					{
						PlaySound(NULL, NULL, SND_ASYNC);
						PlaySound(TEXT("sounds/jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
					}
					player->jumping(true);
				}
				// Allows the player to climb out of the tunnel, when he reaches the top of the ladder and press the SPACE_BAR
				if (player->isClimbing() && isAbleToClimbOut(player))
				{
					player->look(RIGHT);
					player->climbOut(RIGHT);
					PlaySound(NULL, NULL, SND_ASYNC);
					PlaySound(TEXT("sounds/jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
			}
		}
		else
		{
			// Starts the game
			gameStarted = true;
		}
	}
	if (c == 'd' || c == 'D')
	{
		DEBUG_MODE				= !DEBUG_MODE;
		showCollisionRectangles = DEBUG_MODE;	

		if (DEBUG_MODE == false)
		{
			GOD_MODE = false;
		}
		
	}
	if (c == 'g' || c == 'G')
	{
		if (DEBUG_MODE)
		{
			GOD_MODE = !GOD_MODE;
		}		
	}

	if (c == 'l' || c == 'L')
	{
		if (DEBUG_MODE)
		{
			player->setLives(99);
		}
	}

	if (c == 'h' || c == 'H')
	{
		if (DEBUG_MODE)
		{
			showCollisionRectangles = !showCollisionRectangles;
		}
	}
}

void PitfallGame::deleteEnemies()
{
	logs.clear();
	delete scorpion;
	delete snake;
		
	scorpion		= NULL;
	snake			= NULL;
}

void PitfallGame::reset()
{
	world->deleteWorld();	
	this->deleteEnemies();	
	scenarioNumber = 0;
	score = 2000;

	this->spawnEnemies();
	world->buildScenario(0);

	player->resetLives();
	player->respawn();

	
	player->setY(140);	

	world->blackHole->enableSizeChanging();
	world->water->enableSizeChanging();

	paused = false;
}

bool  PitfallGame::isInside(Point& p, Rect& rect)
{
	if (p.x() < rect.rightX() && p.x() > rect.x() && p.y() > rect.y() && p.y() < rect.topY())
	{
		return true;
	}
	return false;
}

bool PitfallGame::isInside(Rect& rect1, Rect& rect2)
{
	if (rect1.rightX() > rect2.x() + 2		// If the right border of the ball is inside the pin
		&& rect1.x() < rect2.rightX() -2    // If the left border of the ball is inside the pin
		&& rect1.topY() > rect2.y() + 1				// If the top border of the ball is inside the pin
		&& rect1.y() < rect2.topY() - 1)
	{
		return true;
	}
	return false;
}

bool PitfallGame::areClose(Player* player, Sprite* object)
{
	if (player->y() < object->topY())
	{
		if (player->rightX() > object->x() - 11 && player->x() < object->rightX() + 11)
		{
			return true;
		}
		return false;
	}
	
	return false;
}

bool PitfallGame::isPaused()
{
	return paused;
}

void PitfallGame::drawCollisionRectangles()
{	
	if ((player->isUndeground()  == false)
		|| player->isUndeground() && thisScenario().nScorpions == 0)
	{
		if (world->hasABonfire() || thisScenario().nSnakes != 0)
		{
			if (world->hasABonfire())
			{
				if (areClose(player, (Sprite*)world->bonfire) == false)
				{
					drawOutline(player->x(), player->y(), player->width(), player->height());
				}
			}
			if (thisScenario().nSnakes != 0)
			{
				if (areClose(player, (Sprite*)snake) == false)
				{
					drawOutline(player->x(), player->y(), player->width(), player->height());
				}
			}
		}
		else
		{
			drawOutline(player->x(), player->y(), player->width(), player->height());
		}		
	}	

	if (thisScenario().nCrocodiles != 0)
	{
		for (unsigned i = 0; i < 3; i++)
		{			
			drawOutline(crocodiles[i].x(), crocodiles[i].y(), crocodiles[i].width(), crocodiles[i].height());
		}
	}

	for (unsigned i = 0; i < logs.size(); i++)
	{
		drawOutline((Sprite)(logs[i]));
	}	

	if (world->hasATreasure())
	{
		drawOutline(*world->treasure);
	}
}

void PitfallGame::drawOutline(Sprite& object)
{
	drawOutline(object.x(), object.y(), object.width(), object.height());
}

void PitfallGame::drawOutline(float x, float y, float width, float height)
{
	glColor3ub(255, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x,y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);	
	glVertex2f(x , y + height);	
	glEnd();
}

ScenarioEnemies& PitfallGame::thisScenario()
{
	if (scenarioNumber >= 0)
	{
		return positiveScenarios.at(scenarioNumber);
	}
	return negativeScenarios.at(abs(scenarioNumber));;
}




