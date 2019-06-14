double pow(double a, double b) {
	union {
		double d;
		int x[2];
	} u = { a };

	u.x[1] = (int) (b * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;

	return u.d;
}
