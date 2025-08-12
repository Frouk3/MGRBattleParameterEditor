#pragma once

namespace BattleParameter
{
    typedef struct
    {
        int m_id;
        int m_AtkPower;
        int m_AtkHavokMulScalar;
        int m_AtkHavokPow;
        int m_HitStopTime;
        int m_Int0;
        int m_Int1;
        float m_BodyAtk;
        float m_BodyDef;
        float m_BodyFcGain;
        float m_BodyFcLoss;
        int m_No;
        float m_EasyPowerScale;
        float m_HardPowerScale;
        float m_VeryhardPowerScale;
        float m_RevengeancePowerScale;
        int m_Int3;
        int m_Int4;
        int m_Int5;
    } Unit;

    typedef struct
    {
        int m_unitCount;
        Unit m_units[];
    } File;
}