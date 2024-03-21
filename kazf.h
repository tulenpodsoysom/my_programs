#ifndef KAZF_H


#include <math.h>
// ����� ��������
template <typename T>
void kazf(T *a, T *b, T *x, int nn, int ny) {
	// nn - ���������� �����������;  ny - ���������� ���������
	double eps = 1.e-6f;
	// double s;
	int i, j, k;
	double s1, s2, fa1, t;
	double *x1;

	x1 = new double[nn];

	x[0] = 0.5f;
	for (i = 1; i < nn; i++)
		x[i] = 0.f;

	s1 = s2 = 1.f;
	while (s1 > eps * s2) {
		for (i = 0; i < nn; i++)
			x1[i] = x[i];

		for (i = 0; i < ny; i++) {
			s1 = 0.0;
			s2 = 0.0;
			for (j = 0; j < nn; j++) {
				fa1 = a[i * nn + j];
				s1 += fa1 * x[j];
				s2 += fa1 * fa1;
			}
			t = (b[i] - s1) / s2;
			for (k = 0; k < nn; k++)
				x[k] += a[i * nn + k] * t;
		}

		s1 = 0.0;
		s2 = 0.0;
		for (i = 0; i < nn; i++) {
			s1 += (x[i] - x1[i]) * (x[i] - x1[i]);
			s2 += x[i] * x[i];
		}
		s1 = (double)sqrt(s1);
		s2 = (double)sqrt(s2);
	}
	delete[] x1;
}

#endif //KAZF_H 
#define KAZF_H