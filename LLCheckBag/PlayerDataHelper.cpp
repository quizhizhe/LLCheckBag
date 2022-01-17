﻿#include "pch.h"
#include "PlayerDataHelper.h"
#include <MC/DBStorage.hpp>
#include <MC/StringTag.hpp>
#include <MC/VanillaBlocks.hpp>
namespace PlayerDataHelper {
    DBHelpers::Category const playerCategory = (DBHelpers::Category)7;
    std::string const PLAYER_KEY_SERVER_ID = "ServerId";
    std::string const PLAYER_KEY_MSA_ID = "MsaId";
    std::string const PLAYER_KEY_SELF_SIGNED_ID = "SelfSignedId";

    void forEachUuid(bool includeSelfSignedId, std::function<void(std::string_view const& uuid)> callback)
    {
        static size_t count;
        static size_t serverCount;
        serverCount = 0;
        count = 0;
        Global<DBStorage>->forEachKeyWithPrefix("player_", playerCategory,
            [&callback, includeSelfSignedId](gsl::cstring_span<-1> key_left, gsl::cstring_span<-1> data) {
                if (key_left.size() == 36) {
                    auto tag = CompoundTag::fromBinaryNBT((void*)data.data(), data.size());
                    auto& msaId = tag->getString(PLAYER_KEY_MSA_ID);
                    //logger.warn("{}", key_left.data());
                    //logger.warn(tag->toJson(4));
                    if (!msaId.empty()) {
                        if (msaId == key_left) {
                            count++;
                            callback(msaId);
                        }
                        return;
                    }
                    if (!includeSelfSignedId)
                        return;
                    auto& selfSignedId = tag->getString(PLAYER_KEY_SELF_SIGNED_ID);
                    //if (msaId.empty() && selfSignedId.empty())
                    //    __debugbreak();
                    if (!selfSignedId.empty()) {
                        if (selfSignedId == key_left) {
                            count++;
                            callback(selfSignedId);
                        }
                        return;
                    }
                }
                else if(key_left.size() == 36+7) {
                    serverCount++;
                }
            });
        if (serverCount != count) {
            logger.warn("获取玩家id数据时发现异常数据");
            //logger.error("ServerId1");
            //for (auto& id : allServerId) {
            //    if (std::find(allServerId2.begin(), allServerId2.end(), id) == allServerId2.end())
            //        logger.warn(id);
            //}
            //logger.error("ServerId2");
            //for (auto& id : allServerId2) {
            //    if (allServerId.find(id) == allServerId.end())
            //        logger.warn(id);
            //}
            //__debugbreak();
        }
    }

    std::vector<string> getAllUuid(bool includeSelfSignedId)
    {
        std::vector<std::string> uuids;
        std::vector<std::string> serverIds;
        forEachUuid(includeSelfSignedId, [&uuids,&serverIds](std::string_view uuid) {
            //auto u = mce::UUID::fromString(std::string(uuid));
            //auto sid = getServerId(u);
            //if (std::find(serverIds.begin(), serverIds.end(), sid) != serverIds.end())
            //    __debugbreak();
            //serverIds.push_back(sid);
            uuids.push_back(std::string(uuid));
            //logger.info("{}", uuid);
            });
        return uuids;
    }
    std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid) {
        auto& dbStorage = *Global<DBStorage>;
        auto playerKey = "player_" + uuid.asString();
        if (dbStorage.hasKey(playerKey, playerCategory)) {
            return dbStorage.getCompoundTag(playerKey, playerCategory);
        }
        return {};
    }
    bool removeData(mce::UUID const& uuid)
    {
        auto& dbStorage = *Global<DBStorage>;
        auto tag = getPlayerIdsTag(uuid);
        if (!tag) {
            //logger.error("key \"player_{}\" not found in storage", uuid.asString());
            return false;
        }
        for (auto& [key, idTagVariant] : *tag) {
            auto idTag = const_cast<CompoundTagVariant&>(idTagVariant).asStringTag();
            std::string id = idTag->value();
            id = key == PLAYER_KEY_SERVER_ID ? id : "player_" + id;
            auto res = dbStorage.deleteData(id, playerCategory);
            //res->addOnComplete([id](Bedrock::Threading::IAsyncResult<void> const& result) {
            //    if (result.getStatus() == 1)
            //        dbLogger.warn("Remove {} Success", id);
            //    else
            //    {
            //        auto code = result.getError();
            //        dbLogger.error("Remove {} Failed, cause: {}", id, code.message());
            //    }
            //    });
        }
        return true;
    }
    std::string getServerId(mce::UUID const& uuid) {
        auto tag = getPlayerIdsTag(uuid);
        if (!tag)
            return "";
        return tag->getString(PLAYER_KEY_SERVER_ID);
    }
    std::unique_ptr<CompoundTag> getPlayerData(mce::UUID const& uuid)
    {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return {};
        return Global<DBStorage>->getCompoundTag(serverId, playerCategory);
    }
    bool writePlayerData(mce::UUID const& uuid, CompoundTag& data) {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return false;
        Global<DBStorage>->saveData(serverId, data.toBinaryNBT(), playerCategory);
        return true;
    }
    void changeBagTag(CompoundTag& dst, CompoundTag& src) {
        dst.put("Armor", src.get("Armor")->copy());
        dst.put("EnderChestInventory", src.get("EnderChestInventory")->copy());
        dst.put("Inventory", src.get("Inventory")->copy());
        dst.put("Mainhand", src.get("Mainhand")->copy());
        dst.put("Offhand", src.get("Offhand")->copy());
    }
    bool setPlayerBag(Player* player, CompoundTag& data) {
        auto playerTag = player->getNbt();
        changeBagTag(*playerTag, data);
        return player->setNbt(playerTag.get());
        player->refreshInventory();
    }
    bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data) {
        auto playerTag = getPlayerData(uuid);
        changeBagTag(*playerTag, data);
        return writePlayerData(uuid, *playerTag);
    }
}
