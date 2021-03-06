// tfunctional.cpp - test functional routines
// Copyright (c) 2013 KALX, LLC. All rights reserved. No warranty is made.
#include <cmath>
#include <iostream>
#include "functional.h"

using namespace std;
using namespace functional;

#define dimof(x) (sizeof(x)/sizeof(*x))

template<class T>
void test_apply_iterator(void)
{
	T t[] = {1, 2, 3};

	auto i = apply([](T x) { return x*x; }, t);
	ensure (*i == 1);
	ensure (*++i == 4);
	ensure (*++i == 9);
	ensure (i == t + 2);

	auto i1(i);

	auto j = apply([](T x) { return x*x; }, t);
	ensure (*j++ == 1);
	ensure (*j++ == 4);
	ensure (*j++ == 9);
	ensure (j == t + 3);

	T* pt = t;
	for (auto k = apply([](T x) { return x*x; }, t); k != t + 3; ++k, ++pt)
		ensure (*k == (*pt)*(*pt));

	auto l = apply((T(*)(T))exp, t);
	ensure (*l++ == exp(t[0]));
	ensure (*l++ == exp(t[1]));
	ensure (*l++ == exp(t[2]));
}

template<class T>
void test_array_iterator(void)
{
	T t[] = {1, 2, 3};

	array_iterator<T> i1(t, t + 3);
	ensure (i1.begin() == t);
	ensure (i1.end() == t + 3);
	ensure (*i1 == t[0]);
	ensure (*++i1 == t[1]);
	ensure (*i1++ == t[1]);
	ensure (*i1 == t[2]);
	*i1 = 4;
	ensure (*i1 == 4);

	i1 = array_iterator<T>(t, t + 3);
	array_iterator<T> i2(3, t);
	ensure (i1 == i2);
	array_iterator<T> i3(i2);
	ensure (i3 == i2);
	++i1;
	ensure (i1 != i3);
	i1 = i3;
	ensure (i1 == i3);

	ensure (!(i1 < i3));
	ensure (i1 <= i3);
	ensure (i1 >= i3);
	ensure (!(i1 > i3));
	++i3;
	ensure (i1 < i3);

	i1 += 2;
	ensure (*i1 == t[2]);
	i1 = i1 - 1;
	ensure (*i1 == t[1]);
}

void test_stride_iterator(void)
{
	int a[] = {0,1,2,3,4,5,6,7,8,9};
	array_iterator<int> ai(sizeof(a)/sizeof(*a), a);
	stride_iterator<array_iterator<int>> si(ai, 2, 1);
	ensure (*si == 1);
	++si;
	ensure (*si == 3);
	ensure (*++si == 5);
}

template<class T>
void test_basis_spline(void)
{
	T t[] = {0,1,2,3,4,5};
	size_t n = sizeof(t)/sizeof(*t);

	auto f0 = basis_spline_func<T,T>(0,n,t);
	ensure (f0(0,-1) == 0);
	ensure (f0(0,0) == 1);
	ensure (f0(0,1) == 0);

	ensure (f0(1,0) == 0);
	ensure (f0(1,1) == 1);

	ensure (f0(n-2,4) == 1);
	ensure (f0(n-2,4.5) == 1);
	ensure (f0(n-2,5) == 0);
	ensure (f0(n-2,9) == 0);

	auto f1 = basis_spline_func<T,T>(1,n,t);
	for (size_t i = 0; i < n - 2; ++i) {
		ensure (f1(i,i-1) == 0);
		ensure (f1(i,i) == 0);
		ensure (f1(i,i+1) == 1);
		ensure (f1(i,i+2) == 0);
		ensure (f1(i,i+3) == 0);
	}

	auto f2 = basis_spline_func<T,T>(2,n,t);
	auto g2 = basis_spline_struct<T,2>(n, t);
	for (size_t i = 0; i < n - 3; ++i) {
		for (T x = -1; x < 6; x += 0.1) {
			ensure (f2(i, x) == g2(i, x));
		}
	}

	auto g3 = basis_spline_struct<T,1>(n, t);
	T y;
	y = g3.integral<T>(0,0);
	y = g3.integral<T>(0,0.5);
	y = g3.integral<T>(0,1);
	y = g3.integral<T>(0,1.5);
	y = g3.integral<T>(0,2);
	y = g3.integral<T>(0,2.5);
	y = g3.integral<T>(0,3);
	
	y = g3.derivative<T>(0,0);
	y = g3.derivative<T>(0,0.5);
	y = g3.derivative<T>(0,1);
	y = g3.derivative<T>(0,1.5);
	y = g3.derivative<T>(0,2);
	y = g3.derivative<T>(0,2.5);
	y = g3.derivative<T>(0,3);

	T t1[] = {0,0,0,1,1,2};
	auto g4 = basis_spline_struct<T,3>(dimof(t1), t1);
	for (T x = 0; x < 2.2; x += 0.1) {
		y = g4(0,x);
	}
}

template<class T>
void test_derivative(void)
{
	T eps = std::numeric_limits<T>::epsilon();

	ensure (derivative([](T x) { return x; })(1) == 1);
	ensure (fabs(derivative((double(*)(double))exp)(1) - exp(1.)) < sqrt(eps));
}

void test_extrapolate(void)
{
	auto H = extrapolate((double(*)(double))exp, 1., (double(*)(double))log);
	ensure (H(0) == exp(0.));
	ensure (H(1) == exp(1.));
	ensure (H(2) == log(2.));
}

template<class T>
void test_integral(void)
{
	ensure (integral([](T x) { return x; })(1) == .5);
	ensure (fabs(integral([](T x) { return x*x; }, 1000)(1.) - 1./3) < 1e-4);
}

template<class T, class U>
void test_piecewise_polynomial(void)
{
	U t[] = {0,0,0,1,1,2,3};
	size_t n = dimof(t);
	T a[] = {1,1};

	piecewise_polynomial_struct<T,U,dimof(t) - dimof(a) - 1> p(a, basis_spline_struct<U, dimof(t) - dimof(a) - 1>(n, t));
	T y;
	for (T x = -0.1; x < 5; x += 0.1) {
		y = p(x);
	}
}

int
main(void)
{
	try {
		test_apply_iterator<double>();
		test_apply_iterator<float>();
		test_array_iterator<double>();
		test_stride_iterator();
		test_basis_spline<double>();
		test_derivative<double>();
		test_extrapolate();
		test_integral<double>();
//		test_functional_polynomial();
		test_piecewise_polynomial<double,double>();
		test_range();
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;

		return -1;
	}

	return 0;
}
