#include "Vector2.h"

#include "hlt\util.hpp"

#include <cmath>
#include <ostream>
#include <string>

Vector2::Vector2(const double a_X, const double a_Y) {
	m_X = a_X;
	m_Y = a_Y;
}

Vector2::~Vector2() {
}

Vector2 Vector2::operator=(const Vector2& a_Rhs) {
	m_X = a_Rhs.m_X;
	m_Y = a_Rhs.m_Y;
	return *this;
}

Vector2 Vector2::operator*(const double & a_Rhs)const {
	return Vector2(m_X*a_Rhs, m_Y*a_Rhs);
}

Vector2 Vector2::operator/(const double & a_Rhs)const {
	return Vector2(m_X / a_Rhs, m_Y / a_Rhs);
}

Vector2 Vector2::operator+(const Vector2 & a_Rhs)const {
	return Vector2(m_X + a_Rhs.m_X, m_Y + a_Rhs.m_Y);
}

Vector2 Vector2::operator-(const Vector2 & a_Rhs) const {
	return Vector2(m_X - a_Rhs.m_X, m_Y - a_Rhs.m_Y);
}

Vector2::operator std::string() const {
	std::string text;
	text = "(X: " + std::to_string(m_X) + ", "
		+ "Y: " + std::to_string(m_Y) + ")";
	return text;
}

Vector2 Vector2::rotateAroundPoint(Vector2 a_Point, double a_Degrees) {
	float angle = (a_Degrees * M_PI) / 180;
	float s = sin(angle);
	float c = cos(angle);

	Vector2 result = *this - a_Point;

	// rotate point
	float xnew = result.m_X * c - result.m_Y * s;
	float ynew = result.m_X * s + result.m_Y * c;

	// translate point back:
	result.m_X = xnew + a_Point.m_X;
	result.m_Y = ynew + a_Point.m_Y;

	return result;
}

void Vector2::normalize() {
	double vLength = length();
	m_X /= vLength;
	m_Y /= vLength;
}

Vector2 Vector2::normalized() {
	Vector2 vector(m_X, m_Y);
	vector.normalize();
	return vector;
}

double Vector2::length() {
	return sqrt(m_X*m_X + m_Y*m_Y);

}

Vector2 & operator*=(Vector2 & a_Lhs, const double & a_Rhs) {
	a_Lhs.m_X *= a_Rhs;
	a_Lhs.m_Y *= a_Rhs;
	return a_Lhs;
}

Vector2 & operator/=(Vector2 & a_Lhs, const double & a_Rhs) {
	a_Lhs.m_X /= a_Rhs;
	a_Lhs.m_Y /= a_Rhs;
	return a_Lhs;
}

Vector2 & operator+=(Vector2 & a_Lhs, const Vector2 & a_Rhs) {
	a_Lhs.m_X += a_Rhs.m_X;
	a_Lhs.m_Y += a_Rhs.m_Y;
	return a_Lhs;
}

std::ostream & operator<<(std::ostream & a_Out, Vector2 & a_Vector) {
	a_Out << "(X: " << a_Vector.m_X;
	a_Out << ", Y: " << a_Vector.m_Y << ")";
	return a_Out;
}
