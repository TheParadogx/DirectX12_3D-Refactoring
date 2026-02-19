#pragma once
#include<DirectXMath.h>

#ifndef DX12_MACROS
#define DX12_MACROS

// Setter,Getter用マクロ
#define DEFINE_VEC2_PROPERTY(name, member) \
    void Set##name(float x, float y) { member.x = x; member.y = y; } \
    void Set##name(const DirectX::XMFLOAT2& val) { member = val; } \
    DirectX::XMFLOAT2 Get##name() const { return member; }

// 単一変数用（floatとか）
#define DEFINE_PROPERTY(type, name, member) \
    void Set##name(type val) { member = val; } \
    type Get##name() const { return member; }

#endif // !DX12_MACROS