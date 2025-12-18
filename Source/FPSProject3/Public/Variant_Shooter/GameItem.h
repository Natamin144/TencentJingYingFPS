// GameItem.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameItemData.h"
#include "GameItem.generated.h"

class AShooterCharacter; // 前向声明玩家角色类

/**
 * 游戏物品基类（所有具体物品的父类）
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class FPSPROJECT3_API UGameItem : public UObject
{
    GENERATED_BODY()

public:
    UGameItem();

    // ========== 初始化 ==========
    /** 初始化物品（必须调用，绑定数据和持有者） */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual void InitItem(const FGameItemData& InItemData, AShooterCharacter* InOwner = nullptr);

    // ========== 核心逻辑 ==========
    /** 使用物品（子类重写实现具体效果） */
    UFUNCTION(BlueprintNativeEvent, Category = "Game Item")
    bool UseItem();
    virtual bool UseItem_Implementation();

    /** 装备物品（如武器、装备） */
    UFUNCTION(BlueprintNativeEvent, Category = "Game Item")
    bool EquipItem();
    virtual bool EquipItem_Implementation();

    /** 卸载物品 */
    UFUNCTION(BlueprintNativeEvent, Category = "Game Item")
    bool UnEquipItem();
    virtual bool UnEquipItem_Implementation();

    // ========== 堆叠逻辑 ==========
    /** 尝试堆叠物品（返回堆叠后的剩余数量） */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual int32 StackItem(int32 InCountToAdd);

    /** 拆分物品（返回拆分后的新物品） */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual UGameItem* SplitItem(int32 InCountToSplit);

    // ========== 数据访问 ==========
    /** 获取物品数据 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    const FGameItemData& GetItemData() const { return ItemData; }

    /** 获取当前堆叠数量 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    FORCEINLINE int32 GetStackCount() const { return StackCount; }

    /** 设置堆叠数量（检查上限） */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual void SetStackCount(int32 NewCount);

    /** 获取物品持有者 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    FORCEINLINE AShooterCharacter* GetItemOwner() const { return ItemOwner; }

    /** 设置物品持有者 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual void SetItemOwner(AShooterCharacter* NewOwner);

protected:
    /** 物品核心数据 */
    UPROPERTY(BlueprintReadOnly, Category = "Game Item")
    FGameItemData ItemData;

    /** 当前堆叠数量 */
    UPROPERTY(BlueprintReadOnly, Category = "Game Item")
    int32 StackCount = 1;

    /** 物品持有者（玩家角色） */
    UPROPERTY(BlueprintReadOnly, Category = "Game Item")
    AShooterCharacter* ItemOwner = nullptr;

    /** 检查物品是否可使用 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual bool CanUseItem() const;

    /** 检查物品是否可装备 */
    UFUNCTION(BlueprintCallable, Category = "Game Item")
    virtual bool CanEquipItem() const;
};