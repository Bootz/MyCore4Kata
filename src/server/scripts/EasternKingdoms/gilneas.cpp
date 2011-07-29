/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Gilneas City
SD%Complete: 10
SDComment:
SDCategory: Gilneas
EndScriptData */

/* ContentData
TODO
EndContentData */

#include "ScriptPCH.h"

//Phase 1
enum eGilneasCityPhase1
{
    //Quests
    QUEST_LOCKDOWN = 14078,

    //Spells
    SPELL_PHASE_2 = 59073,

    //Say
    SAY_PRINCE_LIAM_GREYMANE_1 = -1638000,
    SAY_PRINCE_LIAM_GREYMANE_2 = -1638001,
    SAY_PRINCE_LIAM_GREYMANE_3 = -1638002,
    DELAY_SAY_PRINCE_LIAM_GREYMANE = 20000, //20 seconds repetition time

    SAY_PANICKED_CITIZEN_1 = -1638016,
    SAY_PANICKED_CITIZEN_2 = -1638017,
    SAY_PANICKED_CITIZEN_3 = -1638018,
    SAY_PANICKED_CITIZEN_4 = -1638019,
    #define DELAY_EMOTE_PANICKED_CITIZEN urand(5000, 15000) //5-15 second time... Using #define cuz CONSTANT = value does use Textual substitution and doesn't allows functions
    #define DELAY_SAY_PANICKED_CITIZEN urand(30000, 120000)

    SAY_GILNEAS_CITY_GUARD_GATE_1 = -1638022,
    SAY_GILNEAS_CITY_GUARD_GATE_2 = -1638023,
    SAY_GILNEAS_CITY_GUARD_GATE_3 = -1638024,
    #define DELAY_SAY_GILNEAS_CITY_GUARD_GATE urand(30000, 120000)
};
//Phase 2
enum eGilneasCityPhase2
{
    //Sounds
    SOUND_SWORD_FLESH = 143,
    SOUND_SWORD_PLATE = 147,
    DELAY_SOUND = 500,
    DELAY_ANIMATE = 2000,

    //Spells
    SPELL_PHASE_4 = 59074,

    //NPCs
    NPC_PRINCE_LIAM_GREYMANE = 34913,
    NPC_GILNEAS_CITY_GUARD = 34916,
    NPC_RAMPAGING_WORGEN_1 = 34884,
    NPC_RAMPAGING_WORGEN_2 = 35660,
    NPC_FRIGHTENED_CITIZEN_1 = 34981,
    NPC_FRIGHTENED_CITIZEN_2 = 35836,
};

/*######
## npc_prince_liam_greymane_phase1
######*/

class npc_prince_liam_greymane_phase1 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase1() : CreatureScript("npc_prince_liam_greymane_phase1") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_prince_liam_greymane_phase1AI (pCreature);
    }

    struct npc_prince_liam_greymane_phase1AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase1AI(Creature *c) : ScriptedAI(c) {}

        uint32 tSay; //Time left for say
        uint32 cSay; //Current Say

        //Evade or Respawn
        void Reset()
        {
            tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; //Reset timer
            cSay = 1; //Start from 1
        }

        //Timed events
        void UpdateAI(const uint32 diff)
        {
            //Out of combat
            if (!me->getVictim())
            {
                //Timed say
                if (tSay <= diff)
                {
                    switch(cSay)
                    {
                        default:
                        case 1:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_1, me);
                            cSay++;
							break;
                        case 2:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_2, me);
                            cSay++;
							break;
                        case 3:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_3, me);
                            cSay = 1; //Reset on 1
							break;
                    }

                    tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; //Reset the timer
                }
                else
                {
                    tSay -= diff;
                }
				return;
            }
        }
    };
};

/*######
## npc_panicked_citizen
######*/

uint32 guid_panicked_nextsay = 0; //GUID of the Panicked Citizen that will say random text, this is to prevent more than 1 npc speaking
uint32 tSay_panicked = DELAY_SAY_PANICKED_CITIZEN; //Time left to say
class npc_panicked_citizen : public CreatureScript
{
public:
    npc_panicked_citizen() : CreatureScript("npc_panicked_citizen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_panicked_citizenAI (pCreature);
    }

    struct npc_panicked_citizenAI : public ScriptedAI
    {
        npc_panicked_citizenAI(Creature *c) : ScriptedAI(c) {}

        uint32 tEmote; //Time left for doing an emote

        //Evade or Respawn
        void Reset()
        {
            if (me->GetPhaseMask() == 1)
            {
                tEmote = DELAY_EMOTE_PANICKED_CITIZEN; //Reset timer
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0); //Reset emote state
            }
        }

        void UpdateAI(const uint32 diff)
        {
            //Out of combat and in Phase 1
            if (!me->getVictim() && me->GetPhaseMask() == 1)
            {
                //Timed emote
                if(tEmote <= diff)
                {
                    //Do random emote (1, 5, 18, 20, 430)
                    me->HandleEmoteCommand(RAND(
                        EMOTE_ONESHOT_TALK,
                        EMOTE_ONESHOT_EXCLAMATION,
                        EMOTE_ONESHOT_CRY,
                        EMOTE_ONESHOT_BEG,
                        EMOTE_ONESHOT_COWER));

                    tEmote = DELAY_EMOTE_PANICKED_CITIZEN; //Reset timer
                }
                else
                {
                    tEmote -= diff;
                }

                //Randomly select an NPC to say the next random text
                if(!guid_panicked_nextsay)
                {
                    if(urand(0,1))
                    {
                        guid_panicked_nextsay = me->GetGUIDLow();
                    }
                }

                //If this is the selected npc to say
                if(guid_panicked_nextsay == me->GetGUIDLow())
                {
                    //Timed say
                    if(tSay_panicked <= diff)
                    {
                        //Say random
                        DoScriptText(RAND(
                            SAY_PANICKED_CITIZEN_1,
                            SAY_PANICKED_CITIZEN_2,
                            SAY_PANICKED_CITIZEN_3,
                            SAY_PANICKED_CITIZEN_4),
                        me);

                        guid_panicked_nextsay = 0; //Reset Selected next NPC
                        tSay_panicked = DELAY_SAY_PANICKED_CITIZEN; //Reset timer
                    }
                    else
                    {
                        tSay_panicked -= diff;
                    }
                }
            }
        }
    };
};

/*######
## npc_lieutenant_walden
######*/

class npc_lieutenant_walden : public CreatureScript
{
public:
    npc_lieutenant_walden() : CreatureScript("npc_lieutenant_walden") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lieutenant_waldenAI (pCreature);
    }

    struct npc_lieutenant_waldenAI : public ScriptedAI
    {
        npc_lieutenant_waldenAI(Creature *c) : ScriptedAI(c) {}

        void sQuestReward(Player *pPlayer, const Quest *pQuest, uint32 data)
        {
            if (pQuest->GetQuestId() == QUEST_LOCKDOWN && pPlayer->GetPhaseMask() == 1)
                pPlayer->SetAuraStack(SPELL_PHASE_2, pPlayer, 1); //phaseshift
        }
    };
};

/*######
## npc_gilneas_city_guard_phase1
######*/
class npc_gilneas_city_guard_phase1 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase1() : CreatureScript("npc_gilneas_city_guard_phase1") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_gilneas_city_guard_phase1AI (pCreature);
    }

    struct npc_gilneas_city_guard_phase1AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase1AI(Creature *c) : ScriptedAI(c) {}

        uint32 tSay; //Time left for say

        //Evade or Respawn
        void Reset()
        {
            if (me->GetUInt32Value(UNIT_FIELD_FLAGS) == 134217728)
            {
                tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; //Reset timer
            }
        }

        void UpdateAI(const uint32 diff)
        {
            //Out of combat and
            if (me->GetUInt32Value(UNIT_FIELD_FLAGS) == 134217728)
            {
                //Timed say
                if(tSay <= diff)
                {
                    //Random say
                    DoScriptText(RAND(
                        SAY_GILNEAS_CITY_GUARD_GATE_1,
                        SAY_GILNEAS_CITY_GUARD_GATE_2,
                        SAY_GILNEAS_CITY_GUARD_GATE_3),
                    me);

                    tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; //Reset timer
                }
                else
                {
                    tSay -= diff;
                }
            }
        }
    };
};

/*######
## npc_gilneas_city_guard_phase2
######*/

class npc_gilneas_city_guard_phase2 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase2() : CreatureScript("npc_gilneas_city_guard_phase2") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_gilneas_city_guard_phase2AI (pCreature);
    }

    struct npc_gilneas_city_guard_phase2AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase2AI(Creature *c) : ScriptedAI(c) {}

        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        bool playSnd;

        void Reset()
        {
            tAnimate = DELAY_ANIMATE;
            dmgCount = 0;
            tSound = DELAY_SOUND;
            playSnd = false;
        }

        void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 1.0f);
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                    me->AI()->AttackStart(enemy);

            if (!UpdateVictim())
                return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSnd = false;
            }

            if (playSnd == true) tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSnd = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }
    };
};

/*######
## npc_prince_liam_greymane_phase2
######*/

class npc_prince_liam_greymane_phase2 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase2() : CreatureScript("npc_prince_liam_greymane_phase2") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_prince_liam_greymane_phase2AI (pCreature);
    }

    struct npc_prince_liam_greymane_phase2AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase2AI(Creature *c) : ScriptedAI(c) {}

        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        bool playSnd;
		char* updateDelay;

        void Reset()
        {
            tAnimate = DELAY_ANIMATE;
            dmgCount = 0;
            tSound = DELAY_SOUND;
            playSnd = false;
        }

		void sGossipHello(Player *pPlayer)
		{
			if ((pPlayer->GetQuestStatus(14094) == QUEST_STATUS_REWARDED) && (pPlayer->GetPhaseMask() == 2))
				pPlayer->SetAuraStack(SPELL_PHASE_4, pPlayer, 1); //phaseshift
		}

        void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 1.0f);
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim()) return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSnd = false;
            }

            if (playSnd == true) tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSnd = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }
    };
};

/*######
## npc_rampaging_worgen
######*/

enum eRampaging_worgen
{
    #define SPELL_ENRAGE 8599
    #define CD_ENRAGE 30000
};

class npc_rampaging_worgen : public CreatureScript
{
public:
    npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_rampaging_worgenAI (pCreature);
    }

    struct npc_rampaging_worgenAI : public ScriptedAI
    {
        npc_rampaging_worgenAI(Creature *c) : ScriptedAI(c) {}

        uint32 tEnrage;
        uint32 dmgCount;
        uint32 tAnimate;
        uint32 tSound;
        bool playSound, willCastEnrage;

        void Reset()
        {
            tEnrage = 0;
            dmgCount = 0;
            tAnimate = DELAY_ANIMATE;
            tSound = DELAY_SOUND;
            playSound = false;
			willCastEnrage = urand(0, 1);
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_GILNEAS_CITY_GUARD || target->GetEntry() == NPC_PRINCE_LIAM_GREYMANE)
                dmgCount ++;
        }

        void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 1.0f);
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 1.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                if (Creature* enemy = me->FindNearestCreature(NPC_PRINCE_LIAM_GREYMANE, 10.0f, true))
                    me->AI()->AttackStart(enemy);

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
					me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            //play attack sound
            if (playSound == true) tSound -= diff;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_PLATE);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
                    tAnimate = DELAY_ANIMATE;
                    playSound = true;
                }
                else
                tAnimate -= diff;
            }
        }
    };
};

class npc_rampaging_worgen2 : public CreatureScript
{
public:
    npc_rampaging_worgen2() : CreatureScript("npc_rampaging_worgen2") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_rampaging_worgen2AI (pCreature);
    }

    struct npc_rampaging_worgen2AI : public ScriptedAI
    {
        npc_rampaging_worgen2AI(Creature *c) : ScriptedAI(c) {}

		uint16 tRun, tEnrage;
        bool onceRun, willCastEnrage;
        float x, y, z;

        void JustRespawned()
        {
			tEnrage = 0;
            tRun = 500;
            onceRun = true;
            x = me->m_positionX+cos(me->m_orientation)*8;
            y = me->m_positionY+sin(me->m_orientation)*8;
            z = me->m_positionZ;
			willCastEnrage = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
			if (tRun <= diff && onceRun)
            {
				me->GetMotionMaster()->MoveCharge(x, y, z, 8);
				onceRun = false;
            }
            else tRun -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff)
            {
                if (me->GetHealthPct() <= 30 && willCastEnrage)
                {
					me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

			DoMeleeAttackIfReady();
		}
    };
};

/*######
## go_merchant_square_door
######*/

enum eMerchant_square_door
{
    #define SUMMON1_TTL 300000
    #define QUEST_EVAC_MERC_SQUA 14098
};

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door") { }

    float x, y, z, wx, wy, angle, tQuestCredit;
	bool opened;
	uint8 spawnKind;
	Player* aPlayer;
	GameObject* pGO;

    bool OnGossipHello(Player *pPlayer, GameObject *pGO)
    {
		if (pPlayer->GetQuestStatus(QUEST_EVAC_MERC_SQUA) == QUEST_STATUS_INCOMPLETE)
		{
			aPlayer = pPlayer;
			opened = 1;
			tQuestCredit = 2000;
			pGO->Use(pPlayer);
			spawnKind = urand(1, 3); //1,2=citizen, 3=citizen&worgen (66%,33%)
			angle=pGO->GetOrientation();
			x=pGO->GetPositionX()-cos(angle)*2;
			y=pGO->GetPositionY()-sin(angle)*2;
			z=pGO->GetPositionZ();
			wx = x-cos(angle)*2;
			wy = y-sin(angle)*2;

			if (spawnKind < 3)
			{
				if (Creature *spawnedCreature = pGO->SummonCreature(NPC_FRIGHTENED_CITIZEN_1,x,y,z,angle,TEMPSUMMON_TIMED_DESPAWN,SUMMON1_TTL))
				{
					spawnedCreature->SetPhaseMask(6, 1);
					spawnedCreature->Respawn(1);
				}
			}
			else
			{
				if (Creature *spawnedCreature = pGO->SummonCreature(NPC_FRIGHTENED_CITIZEN_2,x,y,z,angle,TEMPSUMMON_TIMED_DESPAWN,SUMMON1_TTL))
				{
					spawnedCreature->SetPhaseMask(6, 1);
					spawnedCreature->Respawn(1);
				}
			}
			return true;
		}
        return false;
    }

	void OnUpdate(GameObject *pGO, uint32 diff)
	{
		if (opened == 1)
		{
			if (tQuestCredit <= ((float)diff/8))
			{
				opened = 0;
				aPlayer->KilledMonsterCredit(35830, 0);
				if (spawnKind == 3)
				{
					if (Creature *spawnedCreature = pGO->SummonCreature(NPC_RAMPAGING_WORGEN_2,wx,wy,z,angle,TEMPSUMMON_TIMED_DESPAWN,SUMMON1_TTL))
					{
						spawnedCreature->SetPhaseMask(6, 1);
						spawnedCreature->Respawn(1);
						spawnedCreature->getThreatManager().resetAllAggro();
						aPlayer->AddThreat(spawnedCreature, 1.0f);
						spawnedCreature->AddThreat(aPlayer, 1.0f);
					}
				}
			}
			else tQuestCredit -= ((float)diff/8);
		}
	}
};

/*######
## npc_frightened_citizen
######*/

enum eFrightened_citizen
{
    SAY_CITIZEN_1 = -1638003,
    SAY_CITIZEN_2 = -1638004,
    SAY_CITIZEN_3 = -1638005,
    SAY_CITIZEN_4 = -1638006,
    SAY_CITIZEN_5 = -1638007,
    SAY_CITIZEN_6 = -1638008,
    SAY_CITIZEN_7 = -1638009,
	SAY_CITIZEN_8 = -1638010,
	SAY_CITIZEN_1b = -1638011,
	SAY_CITIZEN_2b = -1638012,
	SAY_CITIZEN_3b = -1638013,
	SAY_CITIZEN_4b = -1638014,
	SAY_CITIZEN_5b = -1638015,
};

struct Point
{
	float x, y;
};

Point a[] = {{-1537.26f, 1425.83f}, {-1550.26f, 1423.25f}, {-1552.31f, 1393.49f}, {-1557.81f, 1364.3f}, {-1559.74f, 1321.89f}, {-1578.59f, 1317.46f}};
Point b[] = {{-1463.98f, 1427.69f}, {-1437.21f, 1423.68f}, {-1420.65f, 1420.79f}, {-1405.11f, 1416.85f}, {-1402.88f, 1403.74f}, {-1406.78f, 1374.38f}, {-1502.84f, 1341.09f}, {-1531.17f, 1331.84f}, {-1566.86f, 1320.21f}, {-1578.69f, 1317.54f}};

class npc_frightened_citizen : public CreatureScript
{
public:
    npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_frightened_citizenAI (pCreature);
    }

    struct npc_frightened_citizenAI : public ScriptedAI
    {
        npc_frightened_citizenAI(Creature *c) : ScriptedAI(c) {}

		uint8 pointNumberA, pointNumberB;
        uint16 tRun, tRun2, tRun3, tSay;
        bool onceRun, onceRun2, onceRun3, onceSay, way;
        float x, y, z, x2, y2, distA, distB;

		void MultiDistanceMeter(Point *p, uint8 numberOfPoints, float *dist)
		{
			for (uint8 i = 0; i <= (numberOfPoints-1); i++)
			{
				dist[i] = me->GetDistance2d(p[i].x, p[i].y);
			}
		}

		uint8 NearestPoint(Point *p, uint8 numberOfPoints, float *nearestDist)
		{
			float dist[16];
			uint8 pointNumber;
			MultiDistanceMeter(p, numberOfPoints, dist);
			for (uint8 i = 0; i <= numberOfPoints-1; i++)
			{
				if (i == 0)
				{
					*nearestDist = dist[i];
					pointNumber = i;
				}
				else if (*nearestDist > dist[i])
				{
					*nearestDist = dist[i];
					pointNumber = i;
				}
			}
			return pointNumber;
		}

		bool GetWay(Point *a, uint8 numberOfPointsA, Point *b, uint8 numberOfPointsB)
		{
			NearestPoint(a, numberOfPointsA, &distA);
			NearestPoint(b, numberOfPointsB, &distB);
			if (distA < distB) return true;
			else return false;
		}

        void JustRespawned()
        {
            tRun = 500;
			tRun2 = 2500;
			tRun3 = 3000;
			tSay = 1500;
            onceRun = onceRun2 = onceSay = onceRun3 = true;
            x = me->m_positionX+cos(me->m_orientation)*5;
            y = me->m_positionY+sin(me->m_orientation)*5;
            z = me->m_positionZ;
			x2 = x+cos(me->m_orientation)*5;
            y2 = y+sin(me->m_orientation)*5;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tRun <= diff && onceRun)
            {
				me->GetMotionMaster()->MoveCharge(x, y, z, 8);
				onceRun = false;
            }
            else tRun -= diff;

			if (tSay <= diff && onceSay)
            {
				if (me->GetEntry() == NPC_FRIGHTENED_CITIZEN_1)
					DoScriptText(RAND(SAY_CITIZEN_1, SAY_CITIZEN_2, SAY_CITIZEN_3, SAY_CITIZEN_4, SAY_CITIZEN_5, SAY_CITIZEN_6, SAY_CITIZEN_7, SAY_CITIZEN_8), me);
				else
					DoScriptText(RAND(SAY_CITIZEN_1b, SAY_CITIZEN_2b, SAY_CITIZEN_3b, SAY_CITIZEN_4b, SAY_CITIZEN_5b), me);
				onceSay = false;
            }
            else tSay -= diff;

			if (tRun2 <= diff && onceRun2)
            {
				me->GetMotionMaster()->MoveCharge(x2, y2, z, 8);
				onceRun2 = false;
            }
            else tRun2 -= diff;

			if (tRun3 <= diff && onceRun3)//TO DO: optimize //Kupker
            {
				pointNumberA = NearestPoint(a, 6, &distA);
				pointNumberB = NearestPoint(b, 10, &distB);
				way = GetWay(a, 6, b, 10);
				if (way) (me->GetMotionMaster()->MoveCharge(a[pointNumberA].x, a[pointNumberA].y, z, 8));
				else (me->GetMotionMaster()->MoveCharge(b[pointNumberB].x, b[pointNumberB].y, z, 8));
				onceRun3 = false;
			}
			else tRun3 -= diff;

			if (!onceRun3)
			{
				if (way)
				{
					if (pointNumberA <= 5 && (me->GetDistance2d(a[pointNumberA].x, a[pointNumberA].y) >= 1))
					{
						me->GetMotionMaster()->MoveCharge(a[pointNumberA].x, a[pointNumberA].y, z, 8);
					}
					else pointNumberA ++;
					if (pointNumberA >= 6) me->DespawnOrUnsummon();
				}
				else
				{
					if (pointNumberB <= 9 && (me->GetDistance2d(b[pointNumberB].x, b[pointNumberB].y) >= 1))
					{
						me->GetMotionMaster()->MoveCharge(b[pointNumberB].x, b[pointNumberB].y, z, 8);
					}
					else pointNumberB ++;
					if (pointNumberB >= 10) me->DespawnOrUnsummon();
				}
			}
		}
    };
};

void AddSC_gilneas()
{
    new npc_gilneas_city_guard_phase1();
    new npc_gilneas_city_guard_phase2();
    new npc_prince_liam_greymane_phase1();
    new npc_prince_liam_greymane_phase2();
    new npc_rampaging_worgen();
	new npc_rampaging_worgen2();
    new go_merchant_square_door();
    new npc_frightened_citizen();
    new npc_panicked_citizen();
    new npc_lieutenant_walden();
}