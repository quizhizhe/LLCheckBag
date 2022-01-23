# LLCheckBag
这是 Bedrock Minecraft Server 的背包检查插件，需要 LiteLoader 的前置

## 编译
使用
``
git clone --recursive https://github.com/quizhizhe/LLCheckBag.git
``
拉取项目，用 [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) 打开sln文件，选择，生成->生成LLCheckBag

## 用法
* 指令（可将 llcheckbag 替换成设置的指令别名，默认 llcb）：
* 注：如果目标玩家[Target]未设置则为打开对应 GUI
```
llcheckbag
llcheckbag check/c      [Target]        // 检查玩家背包
llcheckbag remove/rm    [Target]        // 移除玩家数据
llcheckbag op           [Target]        // 设置查包管理员
llcheckbag export/e     [Target] [Type] // 导出玩家数据
llcheckbag exportall    [Type]          // 导出所有玩家数据
llcheckbag rollback/rb                  // 回滚玩家数据（一般查包时崩服后用）
llcheckbag overwrite/ow                 // 覆盖被查包玩家数据
llcheckbag stop/s                       // 停止查包
llcheckbag menu/m                       // 打开查包 GUI 菜单
llcheckbag list/l                       // 列出所有玩家（玩家名称或者玩家UUID）
llcheckbag import/i                     // 导入玩家数据
```

## 部分 GUI 说明
### Check  查看玩家背包
### Import 导入玩家数据
 
## 配置文件(plugins/LLCheckBag/config.json)
```json
{
    // 备份数据格式，可选值：Binary, Snbt
    "BackupDataType": "Binary",
    // 背包备份目录
    "BackupDirectory": "plugins/LLCheckBag/Backup/",
    // 指令别名，输入指令时可用别名执行指令，类似 /teleport 的别名 /tp
    "CommandAlias": "llcb",
    // 是否启用自定义操作者
    "CustomOperator": true,
    // 设置默认的 gui 菜单，可选值：Check, Menu, Import, Export, Delete, ExportAll
    "DefaultScreen": "Check",
    // 导出目录，同时也是导入目录
    "ExportDirectory": "plugins/LLCheckBag/Export/",
    // 不懂请选择默认值
    "MsaIdOnly": false,
    // 仅 CustomOperator 为 true 时有效，设置权限使用指令的玩家的xuid（字符串格式）
    "OperatorXuidList": []
}
```

## 备注
* 如果需要编辑并导入玩家数据请确保你对 nbt 数据格式以及玩家数据结构有一定程度的了解，并先请先备份好你的存档
* 建议使用 [nbt-studio](https://github.com/tryashtar/nbt-studio) 或其他 nbt 编辑工具查看或者编辑导出的玩家 nbt 数据（其实就是千万别用记事本）