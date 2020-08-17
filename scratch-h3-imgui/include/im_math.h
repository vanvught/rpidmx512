#ifndef _IM_MATH_H___
#define _IM_MATH_H___


#define UINT_MAX (INT_MAX * 2U + 1U)
#define FLT_MIN		__FLT_MIN__
#define DBL_MIN		__DBL_MIN__
#define LDBL_MIN	__LDBL_MIN__


#define memchr __builtin_memchr

#define sscanf __builtin_sscanf
#define fmodf  __builtin_fmodf

#define strchr  __builtin_strchr
#define strstr  __builtin_strstr

#define cosf    __builtin_cosf
#define sinf    __builtin_sinf

#define sqrtf   __builtin_sqrtf


template<typename T>
T abs_template(T t)
{
  return t>0 ? t : -t;
}


inline float fabsf(float f) {
    return abs_template(f);
}





#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')
// http://www.leapsecond.com/tools/fast_atof.c
// Do not use this one because the converison is imprecise.
static double fast_atof(const char *p)
{
	int frac;
	double sign, value, scale;

	// Skip leading white space, if any.

	while (white_space(*p)) {
		p += 1;
	}

	// Get sign, if any.

	sign = 1.0;
	if (*p == '-') {
		sign = -1.0;
		p += 1;

	}
	else if (*p == '+') {
		p += 1;
	}

	// Get digits before decimal point or exponent, if any.

	for (value = 0.0; valid_digit(*p); p += 1) {
		value = value * 10.0 + (*p - '0');
	}

	// Get digits after decimal point, if any.

	if (*p == '.') {
		double pow10 = 10.0;
		p += 1;
		while (valid_digit(*p)) {
			value += (*p - '0') / pow10;
			pow10 *= 10.0;
			p += 1;
		}
	}

	// Handle exponent, if any.

	frac = 0;
	scale = 1.0;
	if ((*p == 'e') || (*p == 'E')) {
		unsigned int expon;

		// Get sign of exponent, if any.

		p += 1;
		if (*p == '-') {
			frac = 1;
			p += 1;

		}
		else if (*p == '+') {
			p += 1;
		}

		// Get digits of exponent, if any.

		for (expon = 0; valid_digit(*p); p += 1) {
			expon = expon * 10 + (*p - '0');
		}
		if (expon > 308) expon = 308;

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >= 8) { scale *= 1E8;  expon -= 8; }
		while (expon > 0) { scale *= 10.0; expon -= 1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}



#endif