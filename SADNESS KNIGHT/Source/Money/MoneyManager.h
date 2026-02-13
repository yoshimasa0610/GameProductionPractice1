#pragma once

class MoneyManager
{
public:
    void AddMoney(int amount);
    bool SpendMoney(int amount);
    int  GetMoney() const;
    void SetMoney(int amount);

    void OnPlayerDeath(bool keepMoney);//Player死亡時

    void Draw() const; // UI表示（必要なら）

private:
    int money = 0;
};

extern MoneyManager g_MoneyManager;