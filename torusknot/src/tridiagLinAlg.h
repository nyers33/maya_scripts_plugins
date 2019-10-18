#define restrict  __restrict
void solve_tridiagonal_in_place_destructive(float * restrict const x, const size_t X, const float * restrict const a, const float * restrict const b, float * restrict const c) {
	/*
	solves Ax = v where A is a tridiagonal matrix consisting of vectors a, b, c
	x - initially contains the input vector v, and returns the solution x. indexed from 0 to X - 1 inclusive
	X - number of equations (length of vector x)
	a - subdiagonal (means it is the diagonal below the main diagonal), indexed from 1 to X - 1 inclusive
	b - the main diagonal, indexed from 0 to X - 1 inclusive
	c - superdiagonal (means it is the diagonal above the main diagonal), indexed from 0 to X - 2 inclusive

	Note: contents of input vector c will be modified, making this a one-time-use function (scratch space can be allocated instead for this purpose to make it reusable)
	Note 2: We don't check for diagonal dominance, etc.; this is not guaranteed stable
	*/

	/* index variable is an unsigned integer of same size as pointer */
	size_t ix;

	c[0] = c[0] / b[0];
	x[0] = x[0] / b[0];

	/* loop from 1 to X - 1 inclusive, performing the forward sweep */
	for (ix = 1; ix < X; ix++) {
		const float m = 1.0f / (b[ix] - a[ix] * c[ix - 1]);
		c[ix] = c[ix] * m;
		x[ix] = (x[ix] - a[ix] * x[ix - 1]) * m;
	}

	/* loop from X - 2 to 0 inclusive (safely testing loop condition for an unsigned integer), to perform the back substitution */
	for (ix = X - 1; ix-- > 0;)
		x[ix] = x[ix] - c[ix] * x[ix + 1];
}
