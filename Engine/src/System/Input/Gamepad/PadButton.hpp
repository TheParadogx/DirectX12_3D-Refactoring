#pragma once
#include<winrt/Windows.Gaming.Input.h>

namespace Ecse::System
{
	enum class ePadButton : int
	{
		A = 0,
		B,
		X,
		Y,
		DPadUp,
		DPadDown,
		DPadLeft,
		DPadRight,
		L1,
		R1,
		L2,
		R2,
		L3,
		R3,
		Menu,
		View,
		Paddle1,
		Paddle2,
		Paddle3,
		Paddle4,
		Count
	};

    inline winrt::Windows::Gaming::Input::GamepadButtons ToWgiButton(const ePadButton Button)
    {
        using B = ePadButton;
        using Win = winrt::Windows::Gaming::Input::GamepadButtons;

        switch (Button)
        {
        case B::A: return Win::A;
        case B::B: return Win::B;
        case B::X: return Win::X;
        case B::Y: return Win::Y;
        case B::DPadUp: return Win::DPadUp;
        case B::DPadDown: return Win::DPadDown;
        case B::DPadLeft: return Win::DPadLeft;
        case B::DPadRight: return Win::DPadRight;
        case B::L1: return Win::LeftShoulder;
        case B::R1: return Win::RightShoulder;
        case B::L3: return Win::LeftThumbstick;
        case B::R3: return Win::RightThumbstick;
        case B::Menu: return Win::Menu;
        case B::View: return Win::View;
        case B::Paddle1: return Win::Paddle1;
        case B::Paddle2: return Win::Paddle2;
        case B::Paddle3: return Win::Paddle3;
        case B::Paddle4: return Win::Paddle4;
        default: return Win::None;
        }
    }

    enum class ePadIndex : uint8_t
    {
        Pad1 = 0, Pad2, Pad3, Pad4, Count,
    };
}