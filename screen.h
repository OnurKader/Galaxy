#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

constexpr std::uint16_t WIDTH = 950;
constexpr std::uint16_t HEIGHT = 670;

constexpr std::uint16_t dW = 8;
constexpr std::uint16_t dH = 8;

constexpr std::size_t index(std::size_t row, std::size_t col, std::size_t width = WIDTH)
{
	return (row * width + col);
}

constexpr std::size_t index(int row, int col, std::size_t width = WIDTH)
{
	if(row < 0 || col < 0)
		return 0;

	const auto row_ul = static_cast<std::size_t>(row);
	const auto col_ul = static_cast<std::size_t>(col);

	return (row_ul * width + col_ul);
}

class Screen
{
public:
	explicit Screen(float x, float y, float z) : x(x), y(y), zoom(z)
	{
		Setup();
		Clear();
	}

	void Clear() { std::fill(std::begin(canvas), std::end(canvas), false); }

	void PlotPoint(float x, float y)
	{
		int pos[2];
		transform(pos, x, y);
		drawPoint(pos[0], pos[1]);
	}

	void PlotLine(float x1, float y1, float x2, float y2)
	{
		int pos1[2], pos2[2];
		transform(pos1, x1, y1);
		transform(pos2, x2, y2);
		drawLine(pos1[0], pos1[1], pos2[0], pos2[1]);
	}

	void PlotCircle(float x, float y, float r)
	{
		int p1[2], p2[2];
		transform(p1, x - r, y + r);
		transform(p2, x + r, y - r);

		for(int i = p1[0]; i <= p2[0]; i++)
		{
			for(int j = p1[1]; j <= p2[1]; j++)
			{
				float xt = (float)(j - WIDTH / 2) / zoom + this->x;
				float yt = (float)(HEIGHT / 2 - 1 - i) / zoom + this->y;
				float radius2 = (xt - x) * (xt - x) + (yt - y) * (yt - y);
				if(radius2 <= r * r)
				{
					drawPoint(i, j);
				}
			}
		}
	}

	void PlotRectangle(float x1, float y1, float x2, float y2)
	{
		int p1[2], p2[2];
		transform(p1, x1, y1);
		transform(p2, x2, y2);
		drawRectangle(p1[0], p1[1], p2[0], p2[1]);
	}

	void Position(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	void Zoom(float zoom) { this->zoom = zoom; }

	void Draw()
	{
		// TODO: Change this to a 1D array as well
		char frame[HEIGHT / dH][WIDTH / dW + 1];

		for(int i = 0; i < HEIGHT / dH - 1; ++i)
			frame[i][WIDTH / dW] = '\n';

		frame[HEIGHT / dH - 1][WIDTH / dW] = '\0';

		for(std::size_t i = 0; i < HEIGHT / dH; i++)
		{
			for(std::size_t j = 0; j < WIDTH / dW; j++)
			{
				std::size_t count = 0;

				// calculating brightness
				for(std::size_t k = 0; k < dH; k++)
					for(std::size_t l = 0; l < dW; l++)
						count += canvas[index(dH * i + k, dW * j + l)];

				frame[i][j] = brightness(count);
			}
		}
		// borders
		for(std::size_t i = 0; i < HEIGHT / dH; ++i)
		{
			frame[i][0] = '@';
			frame[i][WIDTH / dW - 1] = '@';
		}
		for(std::size_t j = 0; j < WIDTH / dW; ++j)
		{
			frame[0][j] = '@';
			frame[HEIGHT / dH - 1][j] = '@';
		}

		FillScreenWithString(frame[0]);
	}

	int Height() const { return HEIGHT; }

	int Width() const { return HEIGHT; }

	void set_palette(std::size_t palette) { this->_palette = palette; }

private:
	bool canvas[HEIGHT * WIDTH];
	float x = 0;
	float y = 0;
	float zoom = 1;
	std::size_t _palette = 0;

	void Setup();
	void FillScreenWithString(const char* frame);

	void transform(int pos[2], float x, float y)
	{
		// from world to screen coordinates
		x = (x - this->x) * zoom + (WIDTH / 2);
		y = (y - this->y) * zoom + (HEIGHT / 2);

		pos[0] = (int)(HEIGHT - 1 - y);
		pos[1] = (int)x;
	}

	// drawing functions

	void drawPoint(int A, int B)
	{
		if(A < 0 || B < 0 || A >= HEIGHT || B >= WIDTH)
			return;

		canvas[index(A, B)] = true;
	}

	void drawBoldPoint(int A, int B)
	{
		for(int i = A - 1; i <= A + 1; i++)
		{
			for(int j = B - 1; j <= B + 1; j++)
			{
				drawPoint(i, j);
			}
		}
	}

	void drawLine(int A, int B, int C, int D)
	{
		// sorting
		if(A > C)
		{
			std::swap(A, C);
			std::swap(B, D);
		}
		// algorithm
		if(B == D)
		{
			for(int i = A; i <= C; i++)
			{
				drawBoldPoint(i, B);
			}
			return;
		}
		if(A == C)
		{
			if(D < B)
			{
				std::swap(B, D);
			}
			for(int i = B; i <= D; ++i)
			{
				drawBoldPoint(A, i);
			}
			return;
		}
		if(std::abs(D - B) < std::abs(C - A))
		{
			drawLineLow(A, B, C, D);
		}
		else
		{
			if(B > D)
				drawLineHigh(C, D, A, B);
			else
				drawLineHigh(A, B, C, D);
		}
	}

	void drawRectangle(int i1, int j1, int i2, int j2)
	{
		int minI = i1 < i2 ? i1 : i2;
		int maxI = i1 < i2 ? i2 : i1;
		int minJ = j1 < j2 ? j1 : j2;
		int maxJ = j1 < j2 ? j2 : j1;
		for(int i = minI; i <= maxI; i++)
		{
			for(int j = minJ; j <= maxJ; j++)
			{
				drawPoint(i, j);
			}
		}
	}

	void drawLineLow(int x0, int y0, int x1, int y1)
	{
		int dx = x1 - x0, dy = y1 - y0, yi = 1;
		if(dy < 0)
		{
			yi = -1;
			dy = -dy;
		}
		int D = 2 * dy - dx;
		int y = y0;

		for(int x = x0; x <= x1; x++)
		{
			drawBoldPoint(x, y);
			if(D > 0)
			{
				y += yi;
				D -= 2 * dx;
			}
			D += 2 * dy;
		}
	}

	void drawLineHigh(int x0, int y0, int x1, int y1)
	{
		int dx = x1 - x0, dy = y1 - y0, xi = 1;
		if(dx < 0)
		{
			xi = -1;
			dx = -dx;
		}
		int D = 2 * dx - dy;
		int x = x0;

		for(int y = y0; y <= y1; y++)
		{
			drawBoldPoint(x, y);
			if(D > 0)
			{
				x += xi;
				D -= 2 * dy;
			}
			D += 2 * dx;
		}
	}

	char brightness(std::size_t count) const
	{
		static const struct
		{
			std::size_t n;
			const char s[11];
		} p[] = {
			{10, " .,:;oOQ#@"},
			{10, "     .oO@@"},
			{3, " .:"},
		};
		if(_palette <= 2)
		{
			const auto& pal = p[_palette];
			return pal.s[count * (pal.n - 1) / dW / dH];
		}
		else
		{
			return ' ';
		}
	}
};

inline std::uint16_t termHeight = 24;
inline std::uint16_t termWidth = 80;

inline void Screen::FillScreenWithString(const char* frame)
{
	const auto h = std::min(termHeight, static_cast<std::uint16_t>(HEIGHT / dH));
	const auto linewidth = (WIDTH / dW) + 1U;
	for(std::size_t r = 0; r < h; ++r)
	{
		const char* line = &frame[r * linewidth];
		fprintf(stdout, "\033[%zu;1H", r);
		fwrite(line, 1, linewidth, stdout);
	}
	fflush(stdout);
}

inline void Screen::Setup()
{
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	termWidth = ts.ws_col;
	termHeight = ts.ws_row;
}
