// GameItemEnums.h
#pragma once
#include "CoreMinimal.h"
#include "GameItemEnums.generated.h"

/** 物品类型 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
    // 基础类型
    None,
    Consumable,    // 消耗品（血包、蓝药）
    Weapon,        // 武器（枪械、近战）
    Ammo,          // 弹药
    Equipment,     // 装备（头盔、护甲）
    Resource       // 资源（材料、金币）
};

/** 物品品质 */
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common,        // 普通
    Uncommon,      // 稀有
    Rare,          // 史诗
    Legendary      // 传说
};