#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <complex>

class Gabor
{
public:

	Gabor()
	{
	}

	~Gabor()
	{
	}

	float get(glm::vec2 pos)
	{
		return s.get(pos).real() * k.get(pos);
	}

	void setGaussianKernel(glm::vec2 center, glm::vec2 spread, float angle, float amplitude)
	{
		float sXsq = spread.x * spread.x;
		float sYsq = spread.y * spread.y;

		float a =  0.5f  * (pow(cos(angle), 2) / sXsq) + 0.5f  * (pow(sin(angle), 2) / sYsq);
		float b = -0.25f * (sin(2 * angle)     / sXsq) + 0.25f * (sin(2 * angle)     / sYsq);
		float c =  0.5f  * (pow(sin(angle), 2) / sXsq) + 0.5f  * (pow(cos(angle), 2) / sYsq);

		k = GaussianKernel{ center, amplitude, a, b, c };
	}

	void setComplexSinusoid(glm::vec2 spatialOrientation, float spatialFrequency = 1.f)
	{
		s = ComplexSinusoid{ spatialOrientation, spatialFrequency };
	}

private:
	struct GaussianKernel {
		glm::vec2 center;
		float amplitude;
		float a, b, c;

		float get(glm::vec2 pos)
		{
			glm::vec2 d = pos - center;
			return amplitude * exp(	-( a * d.x * d.x - 2.f * b * d.x * d.y + c * d.y * d.y ) );
		}
	} k;

	struct ComplexSinusoid {
		glm::vec2 spatialOrientation;
		float spatialFrequency;

		std::complex<float> get(glm::vec2 pos)
		{
			return exp(glm::two_pi<float>()           // 2pi
				* spatialFrequency
				* std::complex<float>(0.f, 1.f)       // 0 + 1i
				* glm::dot(spatialOrientation, pos));
		}
	} s;
};

