#pragma once

#include <ostream>

class Vector2 {
public:
	Vector2() : Vector2(0, 0) {};
	Vector2(const double a_X,const double a_Y);
	~Vector2();

	Vector2 operator=(const Vector2& a_Rhs);

	const Vector2 operator*(const double& a_Rhs)const;
	const Vector2 operator/(const double& a_Rhs)const;
	const Vector2 operator+(const Vector2& a_Rhs)const;
	friend Vector2& operator*=(Vector2& a_Lhs, const double& a_Rhs);
	friend Vector2& operator/=(Vector2& a_Lhs, const double& a_Rhs);
	friend Vector2& operator+=(Vector2& a_Lhs, const Vector2& a_Rhs);


	friend std::ostream& operator<<(std::ostream &a_Out, Vector2& a_Vector);
	operator std::string() const;

	void normalize();
	Vector2 normalized();

	double m_X,m_Y;
};

