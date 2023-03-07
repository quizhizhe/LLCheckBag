﻿// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include <iostream>
#include <llapi/Global.h>
#include <llapi/EventAPI.h>
#include <llapi/LoggerAPI.h>
#include <llapi/utils/PluginOwnData.h>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/BlockInstance.hpp>
#include <llapi/mc/Block.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Player.hpp>
#include <llapi/mc/ServerPlayer.hpp>
#include <llapi/mc/ItemStack.hpp>
#include <llapi/RegCommandAPI.h>
#include <llapi/LLAPI.h>
#include "Config.h"
#include <llapi/I18nAPI.h>

#endif //PCH_H