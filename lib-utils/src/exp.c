/* https://nic.schraudolph.org/pubs/Schraudolph99.pdf */

#define M_LN2       0.693147180559945309417232121458176568  /* loge(2)        */

#define EXP_A 	(1048576 / M_LN2)
#define EXP_C 	-1

double exp(double x) {
	union {
		double d;
		struct {
			int j, i;
		} n;
	} eco;

	eco.n.i = EXP_A * x + (1072632447 - EXP_C);
	return eco.d;
}
