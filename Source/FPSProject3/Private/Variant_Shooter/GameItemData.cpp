// GameItemData.cpp
#include "Variant_Shooter/GameItemData.h"

// 若用方案A（inline），此步可省略；若用方案B，添加实现：
uint32 GetTypeHash(const FGameItemData& Data)
{
    return GetTypeHash(Data.ItemID);
}