// GameItemData.h
#pragma once
#include "CoreMinimal.h"
#include "GameItemEnums.h"
#include "GameItemData.generated.h"

/** 物品核心数据（序列化友好，可配置） */
USTRUCT(BlueprintType)
struct FPSPROJECT3_API FGameItemData
{
    GENERATED_BODY()

public:
    FGameItemData() = default;

    // ========== 核心标识 ==========
    /** 物品唯一ID（建议用GUID或自增ID，避免重复） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FString ItemID = TEXT("INVALID_ID");

    /** 物品名称（显示用） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FText ItemName = FText::FromString(TEXT("Unnamed Item"));

    /** 物品描述 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (MultiLine = true))
    FText ItemDescription = FText::FromString(TEXT("No description."));

    // ========== 分类/品质 ==========
    /** 物品类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemType ItemType = EItemType::None;

    /** 物品品质 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemRarity ItemRarity = EItemRarity::Common;

    // ========== 可视化 ==========
    /** 物品图标（UI显示） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    UTexture2D* ItemIcon = nullptr;

    /** 物品3D预览模型（物品栏预览） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    UStaticMesh* PreviewMesh = nullptr;

    // ========== 堆叠属性 ==========
    /** 是否可堆叠 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    bool bIsStackable = false;

    /** 最大堆叠数量（仅可堆叠物品有效） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (EditCondition = "bIsStackable"))
    int32 MaxStackCount = 99;

    // ========== 辅助方法 ==========
    /** 检查数据是否有效 */
    bool IsValid() const
    {
        return !ItemID.IsEmpty() && ItemType != EItemType::None;
    }

    /** 重载==运算符（用于物品对比） */
    bool operator==(const FGameItemData& Other) const
    {
        return ItemID == Other.ItemID;
    }
};

// 为FGameItemData添加哈希函数（用于TMap/TSet）
template<>
struct TStructOpsTypeTraits<FGameItemData> : public TStructOpsTypeTraitsBase2<FGameItemData>
{
    enum { WithIdenticalViaEquality = true, WithHashFunction = true };
};