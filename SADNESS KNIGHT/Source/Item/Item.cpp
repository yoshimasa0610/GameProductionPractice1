#include "Item.h"

Item::Item(int id,
    const std::string& name,
    ItemType type,
    int slotCost,
    const BuffEffect& buff,
    const std::string& iconSmallPath,
    const std::string& iconLargePath)
    : id(id),
    name(name),
    type(type),
    slotCost(slotCost),
    buff(buff),
    iconSmallPath(iconSmallPath),
    iconLargePath(iconLargePath)
{
}