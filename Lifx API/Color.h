#pragma once
#include <cstdint>

// These functions taken from http://codesuppository.blogspot.hu/2012/06/hsl2rgb-convert-rgb-colors-to-hsl-or.html

namespace lifx {
	class Color {
	public:
		struct RGBFloats {
			float r,g,b;
		};

		struct RGBBytes {
			uint8_t r,g,b;
		};

		struct HSLBytes {
			uint8_t h,s,l;
		};

		Color(const RGBBytes& bytes) {
			color = bytes;
		}

		Color(const RGBFloats& floats) {
			color.r = (uint8_t) (floats.r * 255);
			color.g =(uint8_t)( floats.g * 255);
			color.b = (uint8_t)(floats.b * 255);
		}

		Color(const HSLBytes& hsl) {
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;

			float L = ((float)hsl.l)/255;
			float S = ((float)hsl.s)/255;
			float H = ((float)hsl.h)/255;

			if(hsl.s == 0)
			{
				color.r = hsl.l;
				color.g = hsl.l;
				color.b = hsl.l;
			}
			else
			{
				float temp1 = 0;
				if(L < .50)
				{
					temp1 = L*(1 + S);
				}
				else
				{
					temp1 = L + S - (L*S);
				}

				float temp2 = 2*L - temp1;

				float temp3 = 0;
				for(int i = 0 ; i < 3 ; i++)
				{
					switch(i)
					{
					case 0: // red
						{
							temp3 = H + .33333f;
							if(temp3 > 1)
								temp3 -= 1;
							HSLtoRGB_Subfunction(r,temp1,temp2,temp3);
							break;
						}
					case 1: // green
						{
							temp3 = H;
							HSLtoRGB_Subfunction(g,temp1,temp2,temp3);
							break;
						}
					case 2: // blue
						{
							temp3 = H - .33333f;
							if(temp3 < 0)
								temp3 += 1;
							HSLtoRGB_Subfunction(b,temp1,temp2,temp3);
							break;
						}
					default:
						{

						}
					}
				}
				color.r = (uint8_t)((((float)r)/100)*255);
				color.g = (uint8_t)((((float)g)/100)*255);
				color.b = (uint8_t)((((float)b)/100)*255);
			}
		}
		HSLBytes GetAsHSL() const
		{
			unsigned int r = color.r;
			unsigned int g = color.g;
			unsigned int b = color.b;

			float r_percent = ((float)r)/255;
			float g_percent = ((float)g)/255;
			float b_percent = ((float)b)/255;

			float max_color = 0;
			if((r_percent >= g_percent) && (r_percent >= b_percent))
			{
				max_color = r_percent;
			}
			if((g_percent >= r_percent) && (g_percent >= b_percent))
				max_color = g_percent;
			if((b_percent >= r_percent) && (b_percent >= g_percent))
				max_color = b_percent;

			float min_color = 0;
			if((r_percent <= g_percent) && (r_percent <= b_percent))
				min_color = r_percent;
			if((g_percent <= r_percent) && (g_percent <= b_percent))
				min_color = g_percent;
			if((b_percent <= r_percent) && (b_percent <= g_percent))
				min_color = b_percent;

			float L = 0;
			float S = 0;
			float H = 0;

			L = (max_color + min_color)/2;

			if(max_color == min_color)
			{
				S = 0;
				H = 0;
			}
			else
			{
				if(L < .50)
				{
					S = (max_color - min_color)/(max_color + min_color);
				}
				else
				{
					S = (max_color - min_color)/(2 - max_color - min_color);
				}
				if(max_color == r_percent)
				{
					H = (g_percent - b_percent)/(max_color - min_color);
				}
				if(max_color == g_percent)
				{
					H = 2 + (b_percent - r_percent)/(max_color - min_color);
				}
				if(max_color == b_percent)
				{
					H = 4 + (r_percent - g_percent)/(max_color - min_color);
				}
			}

			HSLBytes ret;
			ret.h = (uint8_t) (H * 255);
			ret.l = (uint8_t)( L *255);
			ret.s = (uint8_t) (S*255);
			return ret;
		}


		RGBFloats GetAsFloats() const {
			RGBFloats ret;
			ret.r = color.r / 255.f;
			ret.g = color.g / 255.f;
			ret.b = color.b / 255.f;
		}

	protected:
		RGBBytes color;
		static void HSLtoRGB_Subfunction(unsigned int& c, const float& temp1, const float& temp2, const float& temp3)
		{
			if((temp3 * 6) < 1)
				c = (unsigned int)((temp2 + (temp1 - temp2)*6*temp3)*100);
			else
				if((temp3 * 2) < 1)
					c = (unsigned int)(temp1*100);
				else
					if((temp3 * 3) < 2)
						c = (unsigned int)((temp2 + (temp1 - temp2)*(.66666 - temp3)*6)*100);
					else
						c = (unsigned int)(temp2*100);
			return;
		}
	};
}