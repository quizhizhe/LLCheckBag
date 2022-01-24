#include "pch.h"
#include <map>
#include "FormHelper.h"
#include <MC/Packet.hpp>

namespace FormHelper {
    bool sendPlayerListForm(
        Player* player,
        std::string const& title,
        std::string const& content,
        std::function<void(Player* player, mce::UUID const& uuid)>&& callback,
        PlayerCategory category
    ) {
        Form::SimpleForm form(title, content);
        TestFuncTime(CheckBagMgr.getPlayerList, category); // <0.5ms
        auto playerList = CheckBagMgr.getPlayerList(category);
        for (auto& name : playerList) {
            if (player->getRealName() == name) continue;
            form.append(Form::Button(name));
        }
        return form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
            if (index < 0)
                return;
            auto& target = playerList[index];
            auto uuid = mce::UUID::fromString(target);
            if (!uuid) {
                auto suuid = PlayerInfo::getUUID(target);
                uuid = mce::UUID::fromString(suuid);
            }
            callback(player, uuid);
        });
    }

    bool sendPlayerCategoryForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, PlayerCategory category)>&& callback) {
        static std::vector<PlayerCategory> PlayerCategorys{
            PlayerCategory::All,
            PlayerCategory::Normal,
            PlayerCategory::FakePlayer,
            PlayerCategory::Unnamed,
        };

        Form::SimpleForm form(title, tr("screen.player_category.content"));

        for (auto& btn : PlayerCategorys) {
            form.append(Form::Button{ tr("player.category." + toString(btn)) });
        }

        return form.sendTo((ServerPlayer*)player, [title, content, player, callback](int index) {
            if (index < 0)
                return;
            auto category = PlayerCategorys[index];
            callback(player, category);
            });
    }

    bool sendPlayerListWithCategoryForm(
        Player* player,
        std::string const& title,
        std::string const& content,
        std::function<void(Player* player, mce::UUID const& uuid)>&& callback) {
        return sendPlayerCategoryForm(player, tr("operation.start_check"), tr("screen.player_category.content"),
            [](Player* player, PlayerCategory category) {
                sendPlayerListForm(player, tr("operation.start_check"), tr("screen.check.select_target"),
                    [](Player* player, mce::UUID const& uuid) {
                        auto result = CheckBagMgr.startCheckBag(player, uuid);
                        SendCheckResult(result, tr("operation.start_check"));
                    }, category);
            });;
    }

    //bool sendPlayerListFormSlow(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, mce::UUID const& uuid)>&& callback) {
    //    if (Config::GuiWithCategory) {
    //        Form::SimpleForm form(title, "请选择分类");
    //        auto classfiedlayerList = CheckBagMgr.getClassifiedPlayerList();
    //        for (auto& [category, list] : classfiedlayerList) {
    //            auto btn = Form::Button(toString(category));
    //            form.append(btn);
    //        }
    //        return form.sendTo((ServerPlayer*)player, [title, content, player, classfiedlayerList = std::move(classfiedlayerList), callback](int index) {
    //            if (index < 0)
    //                return;
    //            Form::SimpleForm form(title, content);
    //            std::vector<std::string> playerList = classfiedlayerList[index].second;
    //            for (auto& name : playerList) {
    //                auto btn = Form::Button(name);
    //                form.append(btn);
    //            }
    //            form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
    //                if (index < 0)
    //                    return;
    //                auto& target = playerList[index];
    //                auto uuid = mce::UUID::fromString(target);
    //                if (!uuid) {
    //                    auto suuid = PlayerInfo::getUUID(target);
    //                    uuid = mce::UUID::fromString(suuid);
    //                }
    //                callback(player, uuid);
    //            });
    //        });
    //    }
    //    else {
    //        Form::SimpleForm form(title, content);
    //        auto playerList = CheckBagMgr.getPlayerList();
    //        for (auto& name : playerList) {
    //            auto btn = Form::Button(name);
    //            form.append(btn);
    //        }
    //        return form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
    //            if (index < 0)
    //                return;
    //            auto& target = playerList[index];
    //            auto uuid = mce::UUID::fromString(target);
    //            if (!uuid) {
    //                auto suuid = PlayerInfo::getUUID(target);
    //                uuid = mce::UUID::fromString(suuid);
    //            }
    //            callback(player, uuid);
    //        });
    //    }
    //}

    bool sendDataTypeForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, NbtDataType type)>&& callback)
    {
        static std::vector<std::string> DataTypes = {
            toString(NbtDataType::Binary),
            toString(NbtDataType::Snbt),
            toString(NbtDataType::Json),
        };

        Form::SimpleForm form(title, content);
        for (auto& btn : DataTypes) {
            form.append(Form::Button{ btn });
        }
        return form.sendTo((ServerPlayer*)player, [player, callback](int index) {
            if (index < 0)
                return;
            auto dataType = fromString<NbtDataType>(DataTypes[index]);
            callback(player, dataType);
            });
    }

    bool openRemoveDataScreen(Player* player) {
        return sendPlayerListForm(player, tr("operation.remove"), tr("screen.remove.select_target"),
            [](Player* player, mce::UUID const& uuid) {
                auto result = CheckBagMgr.removePlayerData(uuid);
                SendCheckResult(result, tr("operation.remove"));
            });
    }

    bool openCheckBagMenuScreen(Player* player) {
        static std::vector<std::string> CheckBagMenus = {
            "screen.check.menu.check_other",
            "screen.check.menu.update",
            "screen.check.menu.overwrite",
            "screen.check.menu.next",
            "screen.check.menu.stop",
            "screen.check.menu.back",
            "screen.check.menu.remove",
        };

        auto& manager = CheckBagMgr;
        auto uuid = manager.tryGetTargetUuid(player);
        auto name = manager.getNameOrUuid(uuid);
        Form::SimpleForm form(tr("operation.start_check"), tr("screen.check.menu.contet", name));
        for (auto& btn : CheckBagMenus) {
            form.append(Form::Button(tr(btn)));
        }
        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            CheckBagManager::Result result = CheckBagManager::Result::Error;
            switch (do_hash(CheckBagMenus[index].c_str()))
            {
            case do_hash("screen.check.menu.check_other"):
                if (!openCheckBagScreen(player)) {
                    player->sendText(tr("screen.send.error"));
                }
                break;
            case do_hash("screen.check.menu.update"):
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                result = CheckBagMgr.startCheckBag(player, target);
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            }
            case do_hash("screen.check.menu.overwrite"):
                result = CheckBagMgr.overwriteData(player);
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            case do_hash("screen.check.menu.stop"):
                result = CheckBagMgr.stopCheckBag(player);
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            case do_hash("screen.check.menu.remove"):
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                manager.stopCheckBag(player);
                auto result = manager.removePlayerData(target);
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            }
            case do_hash("screen.check.menu.next"):
            {
                // 相对于所有玩家
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                auto targetName = manager.getNameOrUuid(target);
                auto list = manager.getPlayerList();
                auto iter = std::find(list.begin(), list.end(), targetName);
                ASSERT(iter != list.end());
                ++iter;
                if (iter == list.end())
                    iter = list.begin();
                auto result = manager.startCheckBag(player, manager.fromNameOrUuid(*iter));
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            }
            case do_hash("screen.check.menu.back"):
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                auto targetName = manager.getNameOrUuid(target);
                auto list = manager.getPlayerList();
                auto iter = std::find(list.begin(), list.end(), targetName);
                ASSERT(iter != list.end());
                --iter;
                if (iter == list.begin())
                    iter = list.end();
                auto result = manager.startCheckBag(player, manager.fromNameOrUuid(*iter));
                SendCheckResult(result, tr(CheckBagMenus[index]));
                break;
            }
            default:
                break;
            }
            });
    }

    bool openCheckBagScreen(Player* player) {
        if (Config::GuiWithCategory) {
            return sendPlayerListWithCategoryForm(player, tr("operation.start_check"), tr("screen.player_category.content"),
                [](Player* player, mce::UUID const& uuid) {
                    auto result = CheckBagMgr.startCheckBag(player, uuid);
                    SendCheckResult(result, tr("operation.start_check"));
                });
        }
        else {
            return sendPlayerListForm(player, tr("operation.start_check"), tr("screen.check.select_target"),
                [](Player* player, mce::UUID const& uuid) {
                    auto result = CheckBagMgr.startCheckBag(player, uuid);
                    SendCheckResult(result, tr("operation.start_check"));
                });
        }
    }

    bool openCheckBagSmartScreen(Player* player) {
        if (CheckBagMgr.isCheckingBag(player))
            return openCheckBagMenuScreen(player);
        return openCheckBagScreen(player);
    }

    bool openExportScreen(Player* player) {
        return sendPlayerListForm(player, tr("operation.export"), tr("screen.export.select_target"),
            [](Player* player, mce::UUID const& uuid) {
                sendDataTypeForm(player, tr("operation.export"), "",
                    [uuid](Player* player, NbtDataType dataType) {
                        auto result = CheckBagMgr.exportData(uuid, dataType);
                        SendCheckResult(result, tr("operation.export"));
                    });
            });
    }
    inline std::vector<std::string> listdir(std::string const& path) {
        if (!std::filesystem::exists(path))
            return {};
        if (std::filesystem::directory_entry(path).status().type() != std::filesystem::file_type::directory)
            return {};
        std::vector<std::string> listUuid;
        std::vector<std::string> listName;
        for (auto& file : std::filesystem::directory_iterator(path)) {
            auto& filePath = file.path();
            if (filePath.extension() == ".nbt"
                || filePath.extension() == ".snbt") {
                if (filePath.filename().string().find_last_of('.') == 36)
                    listUuid.push_back(filePath.filename().string());
                else
                    listName.push_back(filePath.filename().string());
            }
        }
        std::sort(listUuid.begin(), listUuid.end());
        std::sort(listName.begin(), listName.end(), nameSortFunc);
        for (auto& suuid : listUuid) {
            listName.push_back(suuid);
        }
        return listName;
    }

    bool openImportScreen(Player* player) {
        //return false;
        std::vector<std::string> fileList = listdir(Config::ExportDirectory);
        Form::SimpleForm form(tr("operation.import"), tr("screen.export.select_data"));
        for (auto& btn : fileList) {
            form.append(Form::Button(btn));
        }
        return form.sendTo((ServerPlayer*)player,
            [player, fileList = std::move(fileList)](int index) {
            if (index < 0)
                return;
            std::string fileName = fileList[index];
            std::string filePath = std::filesystem::path(Config::ExportDirectory).append(fileName).string();
            auto nameOrUuid = fileName.substr(0, fileName.find_last_of('.'));
            auto targetUuid = CheckBagManager::fromNameOrUuid(nameOrUuid);
            auto exist = !PlayerDataHelper::getServerId(targetUuid).empty();
            Form::CustomForm form(tr("operation.import"));
            form.append(Form::Label("label", exist ? tr("screen.import.targer_found", nameOrUuid) : tr("screen.import.target_not_found")));
            form.append(Form::Dropdown("importMode", tr("screen.import.mode.text"), { tr("screen.import.mode.bag_only"),tr("screen.import.mode.complete") }));
            //form.append(Form::Dropdown("target", tr("screen.import.target.text"),
            //    exist ? std::vector<std::string>{ tr("screen.import.target.match"), tr("screen.import.target.select") } : std::vector<std::string>{ tr("screen.import.target.select") }));
            form.append(Form::Dropdown("target", tr("screen.import.target.text"),
                { 
                    exist ? tr("screen.import.target.match") : tr("screen.import.target.new"),
                    tr("screen.import.target.select") 
                }));
            form.sendTo((ServerPlayer*)player, 
                [player, filePath, targetUuid](const std::map<string, std::shared_ptr<Form::CustomFormElement>>& data) {
                auto modeDW = std::dynamic_pointer_cast<Form::Dropdown>(data.at("importMode"));
                auto isBagOnly = modeDW->options[modeDW->getData()] == tr("screen.import.mode.bag_only");
                auto targetDW = std::dynamic_pointer_cast<Form::Dropdown>(data.at("target"));
                auto& target = targetDW->options[targetDW->getData()];
                if (target == tr("screen.import.target.match")) {
                    if (getPlayer(targetUuid) && !isBagOnly) {
                        player->sendText(tr("screen.import.error.online"));
                        return;
                    }
                    else {
                        auto result = CheckBagMgr.importData(targetUuid, filePath, isBagOnly);
                        SendCheckResult(result, tr("operation.import"));
                    }
                }
                else if (target == tr("screen.import.target.new")) {
                    if (isBagOnly)
                    {
                        player->sendText("§c§l仅背包模式下不可选择新玩家作为目标§r");
                    }
                    else {
                        auto result = CheckBagMgr.importNewData(filePath);
                        SendCheckResult(result, tr("operation.import"));
                    }
                }
                else if (target == tr("screen.import.target.select")) {
                    sendPlayerListForm(player, tr("screen.import.error.online"), tr("screen.import.select_target"),
                        [filePath, isBagOnly](Player* player, mce::UUID const& uuid) {
                            auto result = CheckBagMgr.importData(uuid, filePath, isBagOnly);
                            SendCheckResult(result, tr("screen.import.error.online"));
                        });
                }
                });
        });
    }

    bool openExportAllScreen(Player* player) {
        return sendDataTypeForm(player, tr("operation.export_all"), tr("operation.export.select_type"),
            [](Player* player, NbtDataType type) {
                auto& manager = CheckBagMgr;
                size_t count = 0;
                for (auto& suuid : manager.getPlayerList()) {
                    auto result = manager.exportData(suuid, type);
                    if (result == CheckBagManager::Result::Success)
                        count++;
                    else {
                        player->sendText(fmt::format("§c§l{}§r", tr("operation.export.failed", suuid, CheckBagManager::getResultString(result))));
                    }
                }
                player->sendText(tr("operation.export.success", count));
            });
    }

    bool openMenuScreen(Player* player) {
        static std::vector<ScreenCategory> MenuButtons{
            ScreenCategory::Check,
            //ScreenCategory::Menu,
            ScreenCategory::Import,
            ScreenCategory::Export,
            ScreenCategory::Delete,
        };

        auto& manager = CheckBagMgr;
        Form::SimpleForm form("LLCheckBag", tr("screen.menu.content"));
        for (auto& btn : MenuButtons) {
            form.append(Form::Button{ tr("screen.category." + toString(btn)) });
        }
        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            switch (MenuButtons[index])
            {
            case ScreenCategory::Check:
                openCheckBagSmartScreen(player);
                break;
            case ScreenCategory::Menu:
                openMenuScreen(player);
                break;
            case ScreenCategory::Import:
                openImportScreen(player);
                break;
            case ScreenCategory::Export:
                openExportScreen(player);
                break;
            case ScreenCategory::Delete:
                openRemoveDataScreen(player);
                break;
            case ScreenCategory::ExportAll: {
                openExportAllScreen(player);
                break;
            }
            default:
                break;
            }
            });
    }
}