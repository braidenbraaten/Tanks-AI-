#pragma once

#include "Vector2.h"
#include <iostream>
#include "Graph.h"
#include "Solver.h"
#include "Grid.h"

#include "IAgent.h"
#include "sfwdraw.h"

/*
    Example dumb agent.

    State Description:
        

        Cannon:
            SCAN:
                Turns Right
                enemy in sight? -> Set Target, FIRE
            FIRE:
                Fire weapon
                -> SCAN

        Tank:
            Explore:
                If touching Target, set random Target
                Path to Target                
*/

class AutoAgent : public IAgent 
{
    // Cache current battle data on each update
    tankNet::TankBattleStateData current;

    // output data that we will return at end of update
    tankNet::TankBattleCommand tbc;

    // Could use this to keep track of the environment, check out the header.
    Grid map;

    //////////////////////
    // States for turret and tank
    enum TURRET { SCAN, AIM, FIRE } turretState = SCAN;
    enum TANK   { SEEK, DODGE     } tankState   = SEEK;

    float randTimer = 0;
	float secondRngTimer = 0;

    // Active target location to pursue
    Vector2 target;

    ///////
    // Scanning
	//scanning using the turret head not the tank body
    void scan()
    {
		bool hasSeenEnemy = false;
		
		randTimer = 1;
		randTimer -= sfw::getDeltaTime();
        Vector2 tf = Vector2::fromXZ(current.cannonForward);  // current forward of your tank
        Vector2 cp = Vector2::fromXZ(current.position); // current position of your tank

        tbc.fireWish = false; //fire the cannon = false
        // Arbitrarily look around for an enemy.
		tbc.cannonMove = tankNet::CannonMovementOptions::LEFT;
			
		

		
        // Search through the possible enemy targets
        for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
            if (current.tacticoolData[aimTarget].inSight && current.canFire) // if in Sight and we can fire
            {
				
                target = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownPosition);
             
				if (dot(tf, normal(target - cp)) >= 0 && dot(tf, normal(target - cp)) < .87f)
				{
					tbc.cannonMove = tankNet::CannonMovementOptions::RIGHT;
				}else if (dot(tf, normal(target - cp)) >= .87f && dot(tf, normal(target - cp)) <= 1.13f)
				{
					tbc.cannonMove = tankNet::CannonMovementOptions::HALT;
					turretState = FIRE;
				}
				else if (dot(tf, normal(target - cp)) < -.1f)
				{
					tbc.cannonMove = tankNet::CannonMovementOptions::LEFT;
					//turretState = FIRE;
				}
			

				//tankState = DODGE;
                break;
			}
			else if (!current.tacticoolData[aimTarget].inSight)
			{
				tbc.cannonMove = tankNet::CannonMovementOptions::RIGHT;
			}
			
    }

	//when you are in sight of the enemy try and dodge / get out of their line of sight 
	//void dodge()
	//{
	//	float rngTimer = 3;

	//	for (int tankAgent = 0; tankAgent < current.playerCount - 1; ++tankAgent)
	//	{
	//		rngTimer -= sfw::getDeltaTime();
	//		Vector2 enemyTankFWD = Vector2::fromXZ(current.tacticoolData[tankAgent].lastKnownCannonForward); // the last know cannon forward position
	//		if (current.tacticoolData[tankAgent].inSight == true)
	//		{
	//			tbc.tankMove = tankNet::TankMovementOptions::RIGHT;
	//		
	//		}
	//		else if (current.tacticoolData[tankAgent].inSight == false)
	//		{
	//			tbc.tankMove = tankNet::TankMovementOptions::FWRD;
	//			rngTimer = rand() % 1;
	//			if (rngTimer < 0)
	//			{

	//				tankState = SEEK;
	//			}
	//			//tankState = SEEK;

	//		}
	//	}
	//}

	//set the player tank ori to be perp to their turret ori so the player can move side to side / strafeing to dodge the shots
	void dodge()
	{
		Vector2 enemyPos;
		Vector2 enemyTurretOri;
		Vector2 playerTankOri = Vector2::fromXZ(current.forward);
		Vector2 playerTurretOri = Vector2::fromXZ(current.cannonForward);

		//tbc.tankMove = tankNet::TankMovementOptions::RIGHT;

		//sets the vectors to the correct values of the enemy
		for (int aimTarget = 0; aimTarget < current.playerCount - 1; ++aimTarget)
		{
			enemyPos = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownPosition);
			enemyTurretOri = Vector2::fromXZ(current.tacticoolData[aimTarget].lastKnownTankForward);
		}

		//angle between your tank body fwd and their tank ori 
	

		tankState = SEEK;

	}

    void fire()
    {
        // No special logic for now, just shoot.
        tbc.fireWish = current.canFire;
        turretState = SCAN;
    }

    std::list<Vector2> path;
    void seek()
    {
        Vector2 cp = Vector2::fromXZ(current.position); // current position
        Vector2 cf = Vector2::fromXZ(current.forward);  // current forward

        
        randTimer -= sfw::getDeltaTime();

        // If we're pretty close to the target, get a new target           
        if (distance(target, cp) < 20 || randTimer < 0)
        {
            target = Vector2::random() * Vector2 { 50, 50 };
            randTimer = rand() % 4 + 2; // every 2-6 seconds randomly pick a new target
        }


       
        // determine the forward to the target (which is the next waypoint in the path)
        Vector2 tf = normal(target - cp);        

		if (dot(cf, tf) > .87f) // if the dot product is close to lining up, move forward
			tbc.tankMove = tankNet::TankMovementOptions::FWRD;
		else
			tbc.tankMove = tankNet::TankMovementOptions::RIGHT;

    }

public:
    tankNet::TankBattleCommand update(tankNet::TankBattleStateData *state)
    {
        current = *state;
        tbc.msg = tankNet::TankBattleMessage::GAME;

        switch (turretState)
        {
        case SCAN: scan(); break;
        case FIRE: fire(); break;
        }

        switch (tankState)
        {
        case SEEK: seek(); break;
		case DODGE: dodge(); break;
        }

        return tbc;
    }
};