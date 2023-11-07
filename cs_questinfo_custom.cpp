#include "RBAC.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "ChatPackets.h"
#include "World.h"
#include "WorldSession.h"
#include "Player.h"
#include "SocialMgr.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SmartEnum.h"

using QuestRelationsIt = QuestRelations::iterator;

//find EnderAndStarter by value
static QuestRelationsIt FindQuestEnderAndStarter(QuestRelations* const relationMap, uint32 quest_id) 
{
    /*std::find_if(relationMap->begin(), relationMap->end(),
        [&](QuestRelations* relationMap) -> bool {
            return relationMap->begin()->second == quest_id;
        });*/

        //todo mb binary search
    for (auto it = relationMap->begin(); it != relationMap->end(); ++it) {
        if (it->second == quest_id) {
            return it;
        }
    }
    return relationMap->end();
}
class questinfo_commandscript_custom : public CommandScript
{
public:
    questinfo_commandscript_custom() : CommandScript("questinfo_commandscript") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> questinfoCommandTable =
        {
            //{ "reward", HandleQuestFullInfoCommand, LANG_COMMAND_QUEST_INFO_FULL, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
            //{ "required", HandleQuestFullInfoCommand, LANG_COMMAND_QUEST_INFO_FULL, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
            //{ "poi", HandleQuestFullInfoCommand, LANG_COMMAND_QUEST_INFO_FULL, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
            //{ "description", HandleQuestFullInfoCommand, LANG_COMMAND_QUEST_INFO_FULL, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
            //{ "console", HandleQuestFullInfoCommand, LANG_COMMAND_QUEST_INFO_FULL, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
            { "", HandleQuestInfoCommand, LANG_COMMAND_QUEST_INFO, rbac::RBAC_ROLE_GAMEMASTER, Trinity::ChatCommands::Console::Yes },
        };

        static std::vector<ChatCommand> commandTable =
        {
            {"questinfo", questinfoCommandTable},
        };

        return commandTable;
    }
    static bool HandleQuestInfoCommand(ChatHandler* handler, uint32 quest_id)
    {
        //!target is needed to calculate xp rewards and money
        Player* target = handler->getSelectedPlayer();
        if (!target) {}
        target = handler->GetPlayer(); //handler->GetSession->GetPlayer(); not suitable for the console
        /*else
            if (!target)
            {
                //handler->SetSentErrorMessage(true);
                return false;
            }*/

        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest_id);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Quest const* nextQuest = sObjectMgr->GetQuestTemplate(quest->GetNextQuestInChain());

        //!comments related to data base are written in Language.h

        handler->PSendSysMessage(LANG_QUESTINFO_NAME, quest->GetTitle());
        handler->PSendSysMessage(LANG_QUESTINFO_ID, quest->GetQuestId());
        //handler->PSendSysMessage(LANG_QUESTINFO_TYPE, quest->GetType());
        handler->PSendSysMessage(LANG_QUESTINFO_LEVEL, quest->GetQuestLevel());
        handler->PSendSysMessage(LANG_QUESTINFO_MIN_LEVEL, quest->GetMinLevel());

        //!checks are needed to avoid outputting null values
        if (nextQuest)handler->PSendSysMessage(LANG_QUESTINFO_REW_NEXT_QUEST, quest->GetNextQuestInChain(), nextQuest ? nextQuest->GetTitle() : "");
        if (target)if (quest->GetXPReward(target))handler->PSendSysMessage(LANG_QUESTINFO_REW_XP, quest->GetXPReward(target)); // (is converted by GetXPReward formula)
        if (target)if (int32(quest->GetRewOrReqMoney(target)))handler->PSendSysMessage(LANG_QUESTINFO_REW_MONEY, int32(quest->GetRewOrReqMoney(target)));
        if (quest->GetRewMoneyMaxLevel())handler->PSendSysMessage(LANG_QUESTINFO_REW_MAXLVLMONEY, quest->GetRewMoneyMaxLevel());
        if (quest->GetRewHonorAddition())handler->PSendSysMessage(LANG_QUESTINFO_REW_HONOR, quest->GetRewHonorAddition());
        if (quest->GetSrcItemId())handler->PSendSysMessage(LANG_QUESTINFO_START_ITEM, quest->GetSrcItemId());
        if (quest->GetPlayersSlain())handler->PSendSysMessage(LANG_QUESTINFO_PLAYER_KILL, quest->GetPlayersSlain());

        uint8 columnNumber = 0;
#pragma region RewardItem and RewardAmount
        if (quest->RewardItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID, columnNumber, quest->RewardItemId[columnNumber++]);
        if (quest->RewardItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID, columnNumber, quest->RewardItemId[columnNumber++]);
        if (quest->RewardItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID, columnNumber, quest->RewardItemId[columnNumber++]);
        if (quest->RewardItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID, columnNumber, quest->RewardItemId[columnNumber++]);
        columnNumber = 0;
        if (quest->RewardItemIdCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID_COUNT, columnNumber, quest->RewardItemIdCount[columnNumber++]);
        if (quest->RewardItemIdCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID_COUNT, columnNumber, quest->RewardItemIdCount[columnNumber++]);
        if (quest->RewardItemIdCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID_COUNT, columnNumber, quest->RewardItemIdCount[columnNumber++]);
        if (quest->RewardItemIdCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID_COUNT, columnNumber, quest->RewardItemIdCount[columnNumber++]);
        if (quest->RewardItemIdCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_ITEMID_COUNT_ALL, quest->GetRewItemsCount()); //count of non-zero RewardAmounts
        columnNumber = 0;
#pragma endregion

#pragma region ItemDrop and ItemDropQuantity
        //        //? for what need ItemDrop?
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID, columnNumber, quest->ItemDrop[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID, columnNumber, quest->ItemDrop[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID, columnNumber, quest->ItemDrop[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID, columnNumber, quest->ItemDrop[columnNumber++]);
        //        columnNumber = 0;
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID_QUANTITY, columnNumber, quest->ItemDropQuantity[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID_QUANTITY, columnNumber, quest->ItemDropQuantity[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID_QUANTITY, columnNumber, quest->ItemDropQuantity[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_ITEM_DROPID_QUANTITY, columnNumber, quest->ItemDropQuantity[columnNumber++]);
        //        columnNumber = 0;
#pragma endregion

#pragma region RewardChoiceItemID and RewardChoiceItemQuantity
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        if (quest->RewardChoiceItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID, columnNumber, quest->RewardChoiceItemId[columnNumber++]);
        columnNumber = 0;
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT, columnNumber, quest->RewardChoiceItemCount[columnNumber++]);
        if (quest->RewardChoiceItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_CHOICE_ITEMID_COUNT_ALL, quest->GetRewChoiceItemsCount()); //count of non-zero RewardChoiceItemCount
        columnNumber = 0;
#pragma endregion

#pragma region RewardTalents and RewardArenaPoints
        if (quest->GetBonusTalents())handler->PSendSysMessage(LANG_QUESTINFO_REW_TALENTS, quest->GetBonusTalents());
        if (quest->GetRewArenaPoints())handler->PSendSysMessage(LANG_QUESTINFO_REW_ARENA_POINTS, quest->GetRewArenaPoints());
#pragma endregion

#pragma region RewardFactionID, RewardFactionValue and RewardFactionOverride
        if (quest->RewardFactionId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_ID, columnNumber, quest->RewardFactionId[columnNumber++]);
        if (quest->RewardFactionId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_ID, columnNumber, quest->RewardFactionId[columnNumber++]);
        if (quest->RewardFactionId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_ID, columnNumber, quest->RewardFactionId[columnNumber++]);
        if (quest->RewardFactionId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_ID, columnNumber, quest->RewardFactionId[columnNumber++]);
        if (quest->RewardFactionId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_ID, columnNumber, quest->RewardFactionId[columnNumber++]);
        columnNumber = 0;
        if (quest->RewardFactionValueId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_VALUE, columnNumber, quest->RewardFactionValueId[columnNumber++]);
        if (quest->RewardFactionValueId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_VALUE, columnNumber, quest->RewardFactionValueId[columnNumber++]);
        if (quest->RewardFactionValueId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_VALUE, columnNumber, quest->RewardFactionValueId[columnNumber++]);
        if (quest->RewardFactionValueId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_VALUE, columnNumber, quest->RewardFactionValueId[columnNumber++]);
        if (quest->RewardFactionValueId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_VALUE, columnNumber, quest->RewardFactionValueId[columnNumber++]);
        columnNumber = 0;
        if (quest->RewardFactionValueIdOverride[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_OVERRIDE, columnNumber, quest->RewardFactionValueIdOverride[columnNumber++]);
        if (quest->RewardFactionValueIdOverride[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_OVERRIDE, columnNumber, quest->RewardFactionValueIdOverride[columnNumber++]);
        if (quest->RewardFactionValueIdOverride[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_OVERRIDE, columnNumber, quest->RewardFactionValueIdOverride[columnNumber++]);
        if (quest->RewardFactionValueIdOverride[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_OVERRIDE, columnNumber, quest->RewardFactionValueIdOverride[columnNumber++]);
        if (quest->RewardFactionValueIdOverride[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REW_FAC_OVERRIDE, columnNumber, quest->RewardFactionValueIdOverride[columnNumber++]);
        columnNumber = 0;
#pragma endregion

#pragma region POI
        //!poi from quest_template (POI - Point Of Interest. POI will be shown on the map when quest is active)
        if (quest->GetPOIContinent())handler->PSendSysMessage(LANG_QUESTINFO_POI_CONTINENT, quest->GetPOIContinent());
        if (quest->GetPOIx())handler->PSendSysMessage(LANG_QUESTINFO_POI_X, quest->GetPOIx());
        if (quest->GetPOIy())handler->PSendSysMessage(LANG_QUESTINFO_POI_Y, quest->GetPOIy());
        if (quest->GetPointOpt())handler->PSendSysMessage(LANG_QUESTINFO_POI_OPT, quest->GetPointOpt());

       
        //!task area(blue blob on the mini-map)
        //handler->PSendSysMessage("Poi area");
        auto poiData = sObjectMgr->GetQuestPOIWrapper(quest->GetQuestId()); //!poi from quest_poi and quest_poi_points
        if (poiData)
        {
            for (auto i = poiData->POIData.QuestPOIBlobDataStats.begin(); i != poiData->POIData.QuestPOIBlobDataStats.end(); ++i)
            {
                handler->PSendSysMessage(LANG_QUESTINFO_AREA_NUM_MAP_ID, i->BlobIndex, i->MapID);
                for (auto j = i->QuestPOIBlobPointStats.begin(); j != i->QuestPOIBlobPointStats.end(); ++j)
                {
                    handler->PSendSysMessage(LANG_QUESTINFO_POI_X_Y, j->X, j->Y);   
                }
            }
        }
#pragma endregion

#pragma region Allow Time & Races
        if (quest->GetTimeAllowed())handler->PSendSysMessage(LANG_QUESTINFO_TIME_ALLOWED, quest->GetTimeAllowed()); //todo
        if (quest->GetAllowableRaces() == 1101)handler->PSendSysMessage("Allowable Races is Alliance");
        else
            if (quest->GetAllowableRaces() == 690)
                handler->PSendSysMessage("Allowable Races is Horde");
            else
                handler->PSendSysMessage(LANG_QUESTINFO_ALLOWABLE_RACES, quest->GetAllowableRaces());
#pragma endregion

#pragma region Description
        //        handler->PSendSysMessage(LANG_QUESTINFO_OBJECTIVES, quest->GetObjectives());
        //        handler->PSendSysMessage(LANG_QUESTINFO_DETAILS, quest->GetDetails());
        //        handler->PSendSysMessage(LANG_QUESTINFO_AREA_DESCRIPTION, quest->GetAreaDescription());
        //        handler->PSendSysMessage(LANG_QUESTINFO_COMPLETED_TEXT, quest->GetCompletedText());
#pragma endregion

#pragma region RequiredNpcOrGo and RequiredNpcOrGoCount
        //todo output if >0 creature if <0 gameobject
        if (quest->RequiredNpcOrGo[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO, columnNumber, quest->RequiredNpcOrGo[columnNumber++]);
        if (quest->RequiredNpcOrGo[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO, columnNumber, quest->RequiredNpcOrGo[columnNumber++]);
        if (quest->RequiredNpcOrGo[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO, columnNumber, quest->RequiredNpcOrGo[columnNumber++]);
        if (quest->RequiredNpcOrGo[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO, columnNumber, quest->RequiredNpcOrGo[columnNumber++]);
        columnNumber = 0;
        if (quest->RequiredNpcOrGoCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO_COUNT, columnNumber, quest->RequiredNpcOrGoCount[columnNumber++]);
        if (quest->RequiredNpcOrGoCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO_COUNT, columnNumber, quest->RequiredNpcOrGoCount[columnNumber++]);
        if (quest->RequiredNpcOrGoCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO_COUNT, columnNumber, quest->RequiredNpcOrGoCount[columnNumber++]);
        if (quest->RequiredNpcOrGoCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_NPC_OR_GO_COUNT, columnNumber, quest->RequiredNpcOrGoCount[columnNumber++]);
        columnNumber = 0;
#pragma endregion

#pragma region RequiredItemId and RequiredItemCount
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        if (quest->RequiredItemId[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_ID, columnNumber, quest->RequiredItemId[columnNumber++]);
        columnNumber = 0;
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        if (quest->RequiredItemCount[columnNumber])handler->PSendSysMessage(LANG_QUESTINFO_REQUIRED_ITEM_COUNT, columnNumber, quest->RequiredItemCount[columnNumber++]);
        columnNumber = 0;
#pragma endregion

#pragma region ObjectiveText
        //        handler->PSendSysMessage(LANG_QUESTINFO_OBJECTIVE_TEXT, columnNumber, quest->ObjectiveText[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_OBJECTIVE_TEXT, columnNumber, quest->ObjectiveText[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_OBJECTIVE_TEXT, columnNumber, quest->ObjectiveText[columnNumber++]);
        //        handler->PSendSysMessage(LANG_QUESTINFO_OBJECTIVE_TEXT, columnNumber, quest->ObjectiveText[columnNumber++]);
        //        columnNumber = 0;
#pragma endregion

#pragma region queststarter/ender
        QuestRelations* relationCreatureStarterMap = sObjectMgr->GetCreatureQuestRelationMapHACK();
        QuestRelations* relationCreatureEnderMap = sObjectMgr->GetCreatureQuestInvolvedRelationMapHACK();
        QuestRelations* relationGOStarterMap = sObjectMgr->GetGOQuestRelationMapHACK();
        QuestRelations* relationGOEnderMap = sObjectMgr->GetGOQuestInvolvedRelationMapHACK();

        const QuestRelationsIt creatureStarterIt = FindQuestEnderAndStarter(relationCreatureStarterMap, quest->GetQuestId());
        const QuestRelationsIt creatureEnderIt = FindQuestEnderAndStarter(relationCreatureEnderMap, quest->GetQuestId());
        const QuestRelationsIt GOStarterIt = FindQuestEnderAndStarter(relationGOStarterMap, quest->GetQuestId());
        const QuestRelationsIt GOEnderIt = FindQuestEnderAndStarter(relationGOEnderMap, quest->GetQuestId());

        if (creatureStarterIt != relationCreatureStarterMap->end())
            handler->PSendSysMessage(LANG_QUESTINFO_CREATURE_STARTER, creatureStarterIt->first);
        if (creatureEnderIt != relationCreatureEnderMap->end())
            handler->PSendSysMessage(LANG_QUESTINFO_CREATURE_ENDER, creatureEnderIt->first);
        if (GOStarterIt != relationGOStarterMap->end())
            handler->PSendSysMessage(LANG_QUESTINFO_GO_STARTER, GOStarterIt->first);
        if (GOEnderIt != relationGOEnderMap->end())
            handler->PSendSysMessage(LANG_QUESTINFO_GO_ENDER, GOEnderIt->first);
#pragma endregion

       
        if (target)handler->PSendSysMessage(LANG_QUESTINFO_STATUS, target->GetQuestStatus(quest->GetQuestId()), EnumUtils::ToTitle(target->GetQuestStatus(quest->GetQuestId())));

        handler->PSendSysMessage(LANG_QUESTINFO_FLAGS, quest->GetFlags());
        for (QuestFlags flag : EnumUtils::Iterate<QuestFlags>())
            if (quest->HasFlag(flag))
                handler->PSendSysMessage("* %s (0x%X)", EnumUtils::ToTitle(flag), flag);

        return true;
    }

private:
};

void AddSC_questinfo_commandscript_custom()
{
    new questinfo_commandscript_custom();
}
