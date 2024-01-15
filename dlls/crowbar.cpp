/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"


#define CROWBAR_BODYHIT_VOLUME 128
#define CROWBAR_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_crowbar, CCrowbar);

void CCrowbar::Spawn()
{
	Precache();
	m_iId = WEAPON_CROWBAR;
	SET_MODEL(ENT(pev), "models/w_crowbar.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CCrowbar::Precache()
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	PRECACHE_SOUND("weapons/cbar_hitbody1.wav");
	PRECACHE_SOUND("weapons/cbar_swing.wav");

	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");
}

bool CCrowbar::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CCrowbar::Deploy()
{
	return DefaultDeploy("models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

void CCrowbar::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}


void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const Vector& mins, const Vector& maxs, edict_t* pEntity)
{
	int i, j, k;
	float distance;
	const Vector* minmaxs[2] = {&mins, &maxs};
	TraceResult tmpTrace;
	Vector vecHullEnd = tr.vecEndPos;
	Vector vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);
	if (tmpTrace.flFraction < 1.0)
	{
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			for (k = 0; k < 2; k++)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i]->x;
				vecEnd.y = vecHullEnd.y + minmaxs[j]->y;
				vecEnd.z = vecHullEnd.z + minmaxs[k]->z;

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0)
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance)
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}


void CCrowbar::PrimaryAttack()
{
	if (!Swing(true))
	{
		SetThink(&CCrowbar::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}


void CCrowbar::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}


void CCrowbar::SwingAgain()
{
	Swing(false);
}


bool CCrowbar::Swing(bool fFirst)
{
	bool fDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 64;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if (fFirst)
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usCrowbar,
			0.0, g_vecZero, g_vecZero, 0, 0, 0,
			0.0, 0, 0.0);
	}


	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(RANDOM_FLOAT(0, 0.1));

			SendWeaponAnim(CROWBAR_ATTACK1MISS);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			m_flTimeWeaponIdle = (9.0 / 35.0);
		}
	}
	else
	{
		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		SendWeaponAnim(CROWBAR_ATTACK1HIT);

		m_flTimeWeaponIdle = (13.0 / 55.0);

#ifndef CLIENT_DLL

		// hit
		fDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		if (m_flPumpTime > 1 || g_pGameRules->IsMultiplayer())
		{
			// first swing does BLISTERING EXPLODALICIOUS damage
			pEntity->TraceAttack(m_pPlayer->pev, 2, gpGlobals->v_forward, &tr, DMG_BULLET|DMG_ALWAYSGIB);
			VectorAdd(m_pPlayer->pev->velocity, gpGlobals->v_forward*-50, m_pPlayer->pev->velocity)
				VectorAdd(pEntity->pev->velocity, gpGlobals->v_forward * 100, pEntity->pev->velocity)
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

			if (pEntity->pev->takedamage && pEntity->BloodColor() != DONT_BLEED)
			{
				for (int asd = 0; asd < 5; asd++) // We get a little SOS
				{
					UTIL_BloodStream(tr.vecEndPos, Vector(RANDOM_LONG(-2550, 2550), RANDOM_LONG(-2550, 2550), RANDOM_LONG(-2550, 2550)), 70, RANDOM_LONG(10, 255));
				}
			}
		}
		else
		{
			// subsequent swings do NONE !!! FUCK YOU !!!!
			pEntity->TraceAttack(m_pPlayer->pev, 0, gpGlobals->v_forward, &tr, DMG_BULLET);

			for (int asd = 0; asd < m_flPumpTime*5; asd++) // Spark bomb
			{
				DecalGunshot(&tr, BULLET_NONE);
			}
		}
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

#endif

		m_flNextPrimaryAttack = GetNextAttackDelay(RANDOM_FLOAT(0.02,0.05));
		m_flPumpTime += m_flNextPrimaryAttack; // Charge it doc

#ifndef CLIENT_DLL
		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool fHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				if (m_flPumpTime < 1)
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "other/hurt2.wav", m_flPumpTime, ATTN_NORM, 0, RANDOM_LONG(90, 110));
				} else {
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/cbar_hitbody1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(90,110));
				}

				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				fHitWorld = false;

				pev->nextthink = gpGlobals->time + 0.5;

			}
		}

		if (m_flPumpTime > 1) {
			m_pPlayer->pev->punchangle = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1));
		} else {
			m_pPlayer->pev->punchangle = Vector(RANDOM_FLOAT(-1, 1) * m_flPumpTime, RANDOM_FLOAT(-1, 1) * m_flPumpTime, RANDOM_FLOAT(-1, 1) * m_flPumpTime);
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			if (m_flPumpTime < 1)
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/cbar_hit2.wav", m_flPumpTime, ATTN_NORM, 0, RANDOM_LONG(90, 110));
			}
			else
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/cbar_hit1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(90, 110));
			}

			pev->nextthink = gpGlobals->time + 0.01;

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif

		SetThink(&CCrowbar::Smack);
	}
	return fDidHit;
}

void CCrowbar::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}
	m_flPumpTime = 0;
	m_flTimeWeaponIdle = (46.0 / 40.0);

	SendWeaponAnim(CROWBAR_IDLE, 0);
}
