#include "MoneyManager.h"
#include "DxLib.h"

MoneyManager g_MoneyManager;

void MoneyManager::AddMoney(int amount)
{
    if (amount > 0) money += amount;
}

bool MoneyManager::SpendMoney(int amount)
{
    if (money < amount) return false;
    money -= amount;
    return true;
}

int MoneyManager::GetMoney() const
{
    return money;
}

void MoneyManager::SetMoney(int amount)
{
    if (amount < 0)
    {
        money = 0;
        return;
    }

    money = amount;
}

void MoneyManager::OnPlayerDeath(bool keepMoney)
{
    int current = g_MoneyManager.GetMoney();
    if (current <= 0) return;

    // 60～80%ロス
    int loseRate = 30; //+GetRand(20); // 60～80
    int lostMoney = current * loseRate / 100;

    if (lostMoney <= 0) return;

    // 所持金減少
    g_MoneyManager.SetMoney(current - lostMoney);

    if (keepMoney)
    {
        // 専用アイテムがある → 今は
        return;
    }

}

void MoneyManager::Draw() const
{
    const int x = 20;
    const int y = 20;

    DrawFormatString(
        x,
        y,
        GetColor(255, 255, 0),
        "お金 : %d",
        money
    );
}