#pragma once

namespace Cr
{
	class NoCopy
	{
		public:
			constexpr NoCopy() = default;
			~NoCopy() = default;

			NoCopy(const NoCopy& other) = delete;
			NoCopy& operator = (const NoCopy& other) = delete;
	};

	class NoMove
	{
		public:
			constexpr NoMove() = default;
			~NoMove() = default;

			NoMove(NoMove&& other) = delete;
			NoMove& operator = (NoMove&& other) = delete;
	};

	class NoValueSemantics : public NoCopy, public NoMove {};
}
