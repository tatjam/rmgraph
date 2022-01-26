#pragma once

template<typename T>
struct Vec2
{
	T x, y;

	Vec2(T nx, T ny)
	{
		x = nx; y = ny;
	}

	Vec2()
	{
		x = 0; y = 0;
	}

	bool operator==(const Vec2& b)
	{
		return x == b.x && y == b.y;
	}

	bool operator!=(const Vec2& b)
	{
		return !(*this == b);
	}

	Vec2 operator+=(const Vec2& b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	Vec2 operator-=(const Vec2& b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	Vec2 operator+(const Vec2& b)
	{
		return Vec2(x + b.x, y + b.y);
	}

	Vec2 operator-(const Vec2& b)
	{
		return Vec2(x - b.x, y - b.y);
	}
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
static const Vec2<int> OUTSIDE = Vec2(-1, -1);

