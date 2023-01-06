#include "Arcanium.h"

namespace DisplaySystem {
    class CS_Display : public CommandScript {
        public:
            CS_Display() : CommandScript("CS_Display") {};

            ChatCommandTable GetCommands() const override {
                static ChatCommandTable displayCommandtable = {
                    { "head",      HandleDispHead,       rbac::RBAC_PERM_COMMAND_DISP_HEAD,        Console::No },
                    { "shoulders", HandleDispShoulders,  rbac::RBAC_PERM_COMMAND_DISP_SHOULDERS,   Console::No },
                    { "shirt",     HandleDispShirt,      rbac::RBAC_PERM_COMMAND_DISP_SHIRT,       Console::No },
                    { "chest",     HandleDispChest,      rbac::RBAC_PERM_COMMAND_DISP_CHEST,       Console::No },
                    { "waist",     HandleDispWaist,      rbac::RBAC_PERM_COMMAND_DISP_WAIST,       Console::No },
                    { "legs",      HandleDispLegs,       rbac::RBAC_PERM_COMMAND_DISP_LEGS,        Console::No },
                    { "feet",      HandleDispFeet,       rbac::RBAC_PERM_COMMAND_DISP_FEET,        Console::No },
                    { "wrists",    HandleDispWrists,     rbac::RBAC_PERM_COMMAND_DISP_WRISTS,      Console::No },
                    { "hands",     HandleDispHands,      rbac::RBAC_PERM_COMMAND_DISP_HANDS,       Console::No },
                    { "back",      HandleDispBack,       rbac::RBAC_PERM_COMMAND_DISP_BACK,        Console::No },
                    { "tabard",    HandleDispTabard,     rbac::RBAC_PERM_COMMAND_DISP_TABARD,      Console::No },
                    { "mainhand",  HandleDispMainhand,   rbac::RBAC_PERM_COMMAND_DISP_MAINHAND,    Console::No },
                    { "offhand",   HandleDispOffhand,    rbac::RBAC_PERM_COMMAND_DISP_OFFHAND,     Console::No },
                    { "list",      HandleDispList,       rbac::RBAC_PERM_COMMAND_DISP_LIST,        Console::No },
                    { "",          HandleDisp,           rbac::RBAC_PERM_COMMAND_DISP,             Console::No },
                };
                static ChatCommandTable commandTable = {
                    { "display", displayCommandtable },
                };
                return commandTable;
            }
            typedef std::vector<ItemBonusEntry const*> ItemBonusList;

            // Display Shared Function
            static bool SetDisplay(ChatHandler* object, char const* args, EquipmentSlots slot, InventoryType type, uint8 multiple) {
                if (!*args) {
                    object->PSendSysMessage("[Display System]: Invalid arguments.");
                    return true;
                }
                Player* player = object->GetSession()->GetPlayer();
                uint32 EntryID = 0;
                uint16 AppearanceID = 0;
                uint16 bonusID = 0;
                // Setting Up Entry(Item)ID
                if (args[0] == '[') {
                    char const* itemNameStr = strtok((char*)args, "]");
                    if (itemNameStr && itemNameStr[0]) {
                        std::string itemName = itemNameStr + 1;
                        auto itr = std::find_if(sItemSparseStore.begin(), sItemSparseStore.end(), [&itemName](ItemSparseEntry const* itemSparse) {
                            for (LocaleConstant i = LOCALE_enUS; i < TOTAL_LOCALES; i = LocaleConstant(i + 1))
                                if (itemName == itemSparse->Display[i])
                                    return true;
                            return false;
                        });

                        if (itr == sItemSparseStore.end()) {
                            object->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, itemNameStr + 1);
                            return false;
                        }
                        EntryID = itr->ID;
                    }
                    else
                        return false;
                }
                else {
                    char const* id = object->extractKeyFromLink((char*)args, "Hitem");
                    if (!id) {
                        object->SendSysMessage("[Display System]: Invalid link or not found Linkable ItemID");
                        return true;
                    }
                    EntryID = atoul(id);
                }
                // Setting up BonusID
                char const* bonus = strtok(NULL, " ");

                if (bonus)
                    bonusID = atoul(bonus);



                Item* OldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (OldItem == NULL)
                {
                    object->SendSysMessage("[Display System]: Equipment slot is empty!");
                    return true;
                }

                if (EntryID == 1)
                {
                    CharacterDatabase.PExecute("DELETE FROM character_item_display WHERE guid = %u", OldItem->GetGUID().GetCounter());
                    player->DeleteFakeEntry(OldItem);
                    return true;
                }

                if (EntryID == 0)
                {
                    CharacterDatabase.PExecute("REPLACE INTO character_item_display(guid, display, appearance, owner) VALUES(%u, %u, %u, %u)", OldItem->GetGUID().GetCounter(), 1, 0, player->GetGUID().GetCounter());
                    player->SetFakeEntry(OldItem, 1, 0);
                    return true;
                }

                if (bonusID > 0)
                {
                    if (ItemBonusList const* itemBonusList = sDB2Manager.GetItemBonusList(bonusID))
                    {
                        for (ItemBonusEntry const* itemBonus : *itemBonusList)
                        {
                            if (itemBonus->Type == 7)
                            {
                                AppearanceID = itemBonus->Value[0];
                                break;
                            }
                        }
                    }
                }

                ItemTemplate const* charitem = sObjectMgr->GetItemTemplate(EntryID);
                if (charitem == 0)
                {
                    object->SendSysMessage("[Display System]: Item not found!");
                    return true;
                }

                InventoryType itemType = charitem->GetInventoryType();
                if (((multiple == 0) && (itemType == type))
                    || ((multiple == 1) && (itemType == INVTYPE_WEAPON || itemType == INVTYPE_2HWEAPON || itemType == INVTYPE_WEAPONMAINHAND || itemType == INVTYPE_WEAPONOFFHAND || itemType == INVTYPE_RANGEDRIGHT || itemType == INVTYPE_RANGED || itemType == INVTYPE_HOLDABLE))
                    || ((multiple == 2) && (itemType == INVTYPE_WEAPON || itemType == INVTYPE_2HWEAPON || itemType == INVTYPE_WEAPONMAINHAND || itemType == INVTYPE_WEAPONOFFHAND || itemType == INVTYPE_SHIELD || itemType == INVTYPE_HOLDABLE || itemType == INVTYPE_RANGED))
                    || ((multiple == 3) && (itemType == INVTYPE_CHEST || itemType == INVTYPE_ROBE || itemType == INVTYPE_BODY)))
                {
                    CharacterDatabase.PExecute("REPLACE INTO character_item_display(guid, display, appearance, owner) VALUES(%u, %u, %u, %u)", OldItem->GetGUID().GetCounter(), EntryID, AppearanceID, player->GetGUID().GetCounter());
                    player->SetFakeEntry(OldItem, EntryID, AppearanceID);

                    return true;
                }
                else
                {
                    object->SendSysMessage("[Display System]: Slots are different.");
                    return true;
                }
                return false;
            }
            // Comands Handlers
            static bool HandleDisp(ChatHandler* object, const char* args) {
                Player* player = object->GetSession()->GetPlayer();
                std::string Args = args;
                object->PSendSysMessage("[Display System]: Type `.help disp` for view subcommand`s list.");
                return true;
            }
            static bool HandleDispOffhand(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_OFFHAND, INVTYPE_WEAPON, 2);
            }
            static bool HandleDispMainhand(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_MAINHAND, INVTYPE_WEAPON, 1);
            }
            static bool HandleDispTabard(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_TABARD, INVTYPE_TABARD, 0);
            }
            static bool HandleDispBack(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_BACK, INVTYPE_CLOAK, 0);
            }
            static bool HandleDispHands(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_HANDS, INVTYPE_HANDS, 0);
            }
            static bool HandleDispWrists(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_WRISTS, INVTYPE_WRISTS, 0);
            }
            static bool HandleDispFeet(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_FEET, INVTYPE_FEET, 0);
            }
            static bool HandleDispLegs(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_LEGS, INVTYPE_LEGS, 0);
            }
            static bool HandleDispWaist(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_WAIST, INVTYPE_WAIST, 0);
            }
            static bool HandleDispChest(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_CHEST, INVTYPE_CHEST, 3);
            }
            static bool HandleDispShirt(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_BODY, INVTYPE_BODY, 3);
            }
            static bool HandleDispShoulders(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_SHOULDERS, INVTYPE_SHOULDERS, 0);
            }
            static bool HandleDispHead(ChatHandler* object, const char* args) {
                return SetDisplay(object, args, EQUIPMENT_SLOT_HEAD, INVTYPE_HEAD, 0);
            }
            static bool HandleDispList(ChatHandler* object, const char* args) {
                Player* player = object->GetPlayer();
                std::string list = "#";
                for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot) {
                    if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot)) {
                        int64 entry = player->GetFakeEntry(item);
                        if (entry == 0)
                            entry = -1;
                        else if (entry == 1)
                            entry = 0;
                        uint16 appearance = 0;
                        if (entry != 0 && entry != -1)
                            appearance = player->GetFakeAppearance(item);
                        list += std::to_string(entry) + " " + std::to_string(appearance) + "|";
                    }
                    else {
                        Item* CharItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                        int64 ID = CharItem->GetEntry();
                        list += ID + " 0|";
                    }
                }

                boost::trim_right(list);
                //NWA_PlayerAnnounce(player, "SMSG:CHAR:DISPLAY:" + list, "NWRPS.DISPLAY");
                return true;
            }

    };
    class PS_Display : public PlayerScript {
    public:
        PS_Display() : PlayerScript("PS_Display") { }

        void OnLogin(Player* player, bool firstlogin) {
            if (!firstlogin) {
                QueryResult result = CharacterDatabase.PQuery("SELECT guid, display, appearance FROM character_item_display WHERE owner = %u", player->GetGUID().GetCounter());
                if (result) {
                    do {
                        Field* field = result->Fetch();
                        ObjectGuid itemGUID = ObjectGuid::Create<HighGuid::Item>(field[0].GetUInt64());
                        uint64 fakeEntry = field[1].GetUInt64();
                        uint16 fakeAppearance = field[2].GetUInt16();
                        if ((fakeEntry == 1 || sObjectMgr->GetItemTemplate(fakeEntry)) && player->GetItemByGuid(itemGUID)) {
                            player->FakeTransmogrificationMap[itemGUID].entry = fakeEntry;
                            player->FakeTransmogrificationMap[itemGUID].appearance = fakeAppearance;
                        }
                    } while (result->NextRow());

                    if (!player->FakeTransmogrificationMap.empty()) {
                        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot) {
                            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot)) {
                                player->SetVisibleItemSlot(slot, item);
                                if (player->IsInWorld())
                                    item->SendUpdateToPlayer(player);
                            }
                        }
                    }
                }
            }
            ChatHandler(player->GetSession()).PSendSysMessage("[Display System]: Type `.display` for setting up your appereance.");

            std::string list = "#";
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot) {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot)) {
                    int64 entry = player->GetFakeEntry(item);
                    if (entry == 0)
                        entry = -1;
                    else if (entry == 1)
                        entry = 0;
                    uint16 appearance = 0;
                    if (entry != 0 && entry != -1)
                        appearance = player->GetFakeAppearance(item);
                    list += std::to_string(entry) + " " + std::to_string(appearance) + "|";
                }
                else {
                    list += "-1 0|";
                }
            }

            boost::trim_right(list);
            //NWA_PlayerAnnounce(player, "SMSG:CHAR:DISPLAY:" + list, "NWRPS.DISPLAY");
        }
        void OnLogout(Player* player) {
            uint64 GUID = player->GetGUID().GetCounter();

            string EquipmentCache = "";

            /*
            // cache equipment...
         for (uint32 i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
         {
             if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
             {
                 ss << uint32(item->GetTemplate()->GetInventoryType()) << ' ' << item->GetDisplayId(this) << ' ';
                 if (SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(item->GetVisibleEnchantmentId(this)))
                     ss << enchant->ItemVisual;
                 else
                     ss << '0';
                 ss << ' '
                     << uint32(sItemStore.AssertEntry(item->GetVisibleEntry(this))->SubclassID) << ' '
                     << uint32(item->GetVisibleSecondaryModifiedAppearanceId(this)) << ' ';
             }
             else
                 ss << "0 0 0 0 0 ";
         }
            */

        }
    };
    class WS_Display : public WorldScript {
    public:
        WS_Display() : WorldScript("WS_Display") { }
        void OnStartup() override {
            CharacterDatabase.DirectExecute("DELETE FROM character_item_display WHERE NOT EXISTS (SELECT 1 FROM item_instance WHERE item_instance.guid = character_item_display.guid)");
        }
    };
};
