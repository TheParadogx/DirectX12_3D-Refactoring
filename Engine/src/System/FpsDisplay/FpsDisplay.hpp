#pragma once

namespace Ecse::System
{
    class FpsDisplay {
    public:
        FpsDisplay() = default;
		static bool StartMonitoring();
    };
} // namespace Ecse::System