/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: GO_Scripts
SD%Complete: 100
SDComment: Quest support: 5097, 5098
SDCategory: Game Objects
EndScriptData

*/

#include "AI/ScriptDevAI/PreCompiledHeader.h"
#include "Entities/Object.h"
/* ContentData
go_andorhal_tower
EndContentData */


/*######
## go_andorhal_tower
######*/

enum
{
    QUEST_ALL_ALONG_THE_WATCHTOWERS_ALLIANCE = 5097,
    QUEST_ALL_ALONG_THE_WATCHTOWERS_HORDE    = 5098,
    NPC_ANDORHAL_TOWER_1                     = 10902,
    NPC_ANDORHAL_TOWER_2                     = 10903,
    NPC_ANDORHAL_TOWER_3                     = 10904,
    NPC_ANDORHAL_TOWER_4                     = 10905,
    GO_ANDORHAL_TOWER_1                      = 176094,
    GO_ANDORHAL_TOWER_2                      = 176095,
    GO_ANDORHAL_TOWER_3                      = 176096,
    GO_ANDORHAL_TOWER_4                      = 176097
};

bool GOUse_go_andorhal_tower(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_ALL_ALONG_THE_WATCHTOWERS_ALLIANCE) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_ALL_ALONG_THE_WATCHTOWERS_HORDE) == QUEST_STATUS_INCOMPLETE)
    {
        uint32 uiKillCredit = 0;
        switch (pGo->GetEntry())
        {
            case GO_ANDORHAL_TOWER_1:   uiKillCredit = NPC_ANDORHAL_TOWER_1;   break;
            case GO_ANDORHAL_TOWER_2:   uiKillCredit = NPC_ANDORHAL_TOWER_2;   break;
            case GO_ANDORHAL_TOWER_3:   uiKillCredit = NPC_ANDORHAL_TOWER_3;   break;
            case GO_ANDORHAL_TOWER_4:   uiKillCredit = NPC_ANDORHAL_TOWER_4;   break;
        }
        if (uiKillCredit)
            pPlayer->KilledMonsterCredit(uiKillCredit);
    }
    return true;
}

/*####
## go_bells
####*/

enum
{
    // Bells
    EVENT_ID_BELLS = 1024
};

enum BellHourlySoundFX
{
    BELLTOLLTRIBAL = 6595, // Horde
    BELLTOLLHORDE = 6675,
    BELLTOLLALLIANCE = 6594, // Alliance
    BELLTOLLNIGHTELF = 6674,
    BELLTOLLDWARFGNOME = 7234
};

enum BellHourlySoundAreas
{
    // Local areas
    TARREN_MILL_AREA = 272,
    KARAZHAN_MAPID   = 532,
    IRONFORGE_1_AREA = 809,
    BRILL_AREA       = 2118,

    // Global areas (both zone and area)
    UNDERCITY_AREA   = 1497,
    STORMWIND_AREA   = 1519,
    IRONFORGE_2_AREA = 1537,
    DARNASSUS_AREA   = 1657,
};

enum BellHourlyObjects
{
    GO_HORDE_BELL    = 175885,
    GO_ALLIANCE_BELL = 176573
};

struct go_ai_bell : public GameObjectAI
{
    go_ai_bell(GameObject* go) : GameObjectAI(go), m_uiBellTolls(0), m_uiBellSound(GetBellSound(go)), m_uiBellTimer(0), m_playTo(GetBellZoneOrArea(go))
    {
        m_go->SetNotifyOnEventState(true);
        m_go->SetActiveObjectState(true);
    }

    uint32 m_uiBellTolls;
    uint32 m_uiBellSound;
    uint32 m_uiBellTimer;
    PlayPacketSettings m_playTo;

    uint32 GetBellSound(GameObject* pGo)
    {
        uint32 soundId;
        switch (pGo->GetEntry())
        {
            case GO_HORDE_BELL:
                switch (pGo->GetAreaId())
                {
                    case UNDERCITY_AREA:
                    case BRILL_AREA:
                    case TARREN_MILL_AREA:
                        soundId = BELLTOLLTRIBAL;
                        break;
                    default:
                        soundId = BELLTOLLHORDE;
                        break;
                }
                break;
            case GO_ALLIANCE_BELL:
            {
                switch (pGo->GetAreaId())
                {
                    case IRONFORGE_1_AREA:
                    case IRONFORGE_2_AREA:
                        soundId = BELLTOLLDWARFGNOME;
                        break;
                    case DARNASSUS_AREA:
                        soundId = BELLTOLLNIGHTELF;
                        break;
                    default:
                        soundId = BELLTOLLALLIANCE;
                        break;
                }
                break;
            }
        }
        return soundId;
    }

    PlayPacketSettings GetBellZoneOrArea(GameObject* pGo)
    {
        PlayPacketSettings playTo = PLAY_AREA;
        switch (pGo->GetEntry())
        {
            case GO_HORDE_BELL:
                switch (pGo->GetAreaId())
                {
                    case UNDERCITY_AREA:
                        playTo = PLAY_ZONE;
                        break;
                }
                break;
            case GO_ALLIANCE_BELL:
            {
                switch (pGo->GetAreaId())
                {
                    case DARNASSUS_AREA:
                    case IRONFORGE_2_AREA:
                        playTo = PLAY_ZONE;
                        break;
                }
                break;
            }
        }
        return playTo;
    }

    void OnEventHappened(uint16 event_id, bool activate, bool resume) override
    {
        if (event_id == EVENT_ID_BELLS && activate && !resume)
        {
            time_t curTime = time(nullptr);
            tm localTm = *localtime(&curTime);
            m_uiBellTolls = (localTm.tm_hour + 11) % 12;

            if (m_uiBellTolls)
                m_uiBellTimer = 3000;

            m_go->GetMap()->PlayDirectSoundToMap(m_uiBellSound, m_go->GetAreaId());
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiBellTimer)
        {
            if (m_uiBellTimer <= uiDiff)
            {
                m_go->PlayDirectSound(m_uiBellSound, PlayPacketParameters(PLAY_AREA, m_go->GetAreaId()));

                m_uiBellTolls--;
                if (m_uiBellTolls)
                    m_uiBellTimer = 3000;
                else
                    m_uiBellTimer = 0;
            }
            else
                m_uiBellTimer -= uiDiff;
        }
    }
};

GameObjectAI* GetAI_go_bells(GameObject* go)
{
    return new go_ai_bell(go);
}

/*####
## go_darkmoon_faire_music
####*/

enum
{
    MUSIC_DARKMOON_FAIRE_MUSIC = 8440
};

struct go_ai_dmf_music : public GameObjectAI
{
    go_ai_dmf_music(GameObject* go) : GameObjectAI(go)
    {
        m_uiMusicTimer = 5000;
    }

    uint32 m_uiMusicTimer;

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiMusicTimer <= uiDiff)
        {
            m_go->PlayMusic(MUSIC_DARKMOON_FAIRE_MUSIC);
            m_uiMusicTimer = 5000;
        }
        else
            m_uiMusicTimer -= uiDiff;
    }
};

GameObjectAI* GetAI_go_darkmoon_faire_music(GameObject* go)
{
    return new go_ai_dmf_music(go);
}

void AddSC_go_scripts()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_andorhal_tower";
    pNewScript->pGOUse =          &GOUse_go_andorhal_tower;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_bells";
    pNewScript->GetGameObjectAI = &GetAI_go_bells;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_darkmoon_faire_music";
    pNewScript->GetGameObjectAI = &GetAI_go_darkmoon_faire_music;
    pNewScript->RegisterSelf();
}
