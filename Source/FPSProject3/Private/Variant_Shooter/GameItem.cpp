// GameItem.cpp
#include "Variant_Shooter/GameItem.h"
#include "ShooterCharacter.h" // 玩家角色类头文件

UGameItem::UGameItem()
{
    // 设置UObject的生命周期（自动垃圾回收）
    SetFlags(RF_StrongRefOnFrame);
}

void UGameItem::InitItem(const FGameItemData& InItemData, AShooterCharacter* InOwner)
{
    ItemData = InItemData;
    ItemOwner = InOwner;
    StackCount = 1; // 默认堆叠数量为1

    UE_LOG(LogTemp, Log, TEXT("物品[%s]初始化完成，持有者：%s"),
        *ItemData.ItemName.ToString(),
        ItemOwner ? *ItemOwner->GetName() : TEXT("None"));
}

bool UGameItem::UseItem_Implementation()
{
    // 基类默认实现：检查是否可使用
    if (!CanUseItem())
    {
        UE_LOG(LogTemp, Warning, TEXT("物品[%s]无法使用"), *ItemData.ItemName.ToString());
        return false;
    }

    // 消耗品：使用后减少堆叠数量，若数量为0则标记为销毁
    if (ItemData.ItemType == EItemType::Consumable)
    {
        SetStackCount(StackCount - 1);
        if (StackCount <= 0)
        {
            // 通知物品栏移除该物品（后续物品栏系统实现）
            if (ItemOwner)
            {
                // ItemOwner->GetInventory()->RemoveItem(this);
            }
        }
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("物品[%s]使用成功"), *ItemData.ItemName.ToString());
    return true;
}

bool UGameItem::EquipItem_Implementation()
{
    if (!CanEquipItem())
    {
        UE_LOG(LogTemp, Warning, TEXT("物品[%s]无法装备"), *ItemData.ItemName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("物品[%s]装备成功"), *ItemData.ItemName.ToString());
    return true;
}

bool UGameItem::UnEquipItem_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("物品[%s]卸载成功"), *ItemData.ItemName.ToString());
    return true;
}

int32 UGameItem::StackItem(int32 InCountToAdd)
{
    if (!ItemData.bIsStackable || InCountToAdd <= 0)
    {
        return InCountToAdd; // 不可堆叠或添加数量无效，返回全部
    }

    const int32 RemainingSpace = ItemData.MaxStackCount - StackCount;
    if (RemainingSpace <= 0)
    {
        return InCountToAdd; // 堆叠已满，返回全部
    }

    const int32 AddCount = FMath::Min(InCountToAdd, RemainingSpace);
    StackCount += AddCount;
    const int32 RemainingCount = InCountToAdd - AddCount;

    UE_LOG(LogTemp, Log, TEXT("物品[%s]堆叠：+%d，当前数量：%d，剩余：%d"),
        *ItemData.ItemName.ToString(), AddCount, StackCount, RemainingCount);

    return RemainingCount;
}

UGameItem* UGameItem::SplitItem(int32 InCountToSplit)
{
    if (!ItemData.bIsStackable || InCountToSplit <= 0 || InCountToSplit >= StackCount)
    {
        return nullptr; // 不可堆叠或拆分数量无效，返回空
    }

    // 创建新物品实例
    UGameItem* NewItem = NewObject<UGameItem>(GetTransientPackage(), GetClass());
    if (NewItem)
    {
        NewItem->InitItem(ItemData, ItemOwner);
        NewItem->SetStackCount(InCountToSplit);
        StackCount -= InCountToSplit;

        UE_LOG(LogTemp, Log, TEXT("物品[%s]拆分：原数量%d → 新数量%d，剩余%d"),
            *ItemData.ItemName.ToString(), InCountToSplit, NewItem->GetStackCount(), StackCount);
    }

    return NewItem;
}

void UGameItem::SetStackCount(int32 NewCount)
{
    StackCount = FMath::Clamp(NewCount, 0, ItemData.MaxStackCount);
}

void UGameItem::SetItemOwner(AShooterCharacter* NewOwner)
{
    ItemOwner = NewOwner;
}

bool UGameItem::CanUseItem() const
{
    return ItemData.IsValid() && ItemOwner != nullptr;
}

bool UGameItem::CanEquipItem() const
{
    return ItemData.IsValid() && ItemOwner != nullptr &&
        (ItemData.ItemType == EItemType::Weapon || ItemData.ItemType == EItemType::Equipment);
}