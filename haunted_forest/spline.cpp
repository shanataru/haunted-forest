//----------------------------------------------------------------------------------------
/**
*      file		|		spline.cpp
*	   source	|		asteriods.cpp - spline.cpp
*/
//----------------------------------------------------------------------------------------

#include "spline.h"
#include "render_stuff.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Checks whether vector is zero-length or not.
bool isVectorNull(glm::vec3& vect)
{
	return !vect.x && !vect.y && !vect.z;
}

/// Align (rotate and move) current coordinate system to given parameters.
/**
This function works similarly to \ref gluLookAt, however it is used for object transform
rather than view transform. The current coordinate system is moved so that origin is moved
to the \a position. Object's local front (-Z) direction is rotated to the \a front and
object's local up (+Y) direction is rotated so that angle between its local up direction and
\a up vector is minimum.

\param[in]  position           Position of the origin.
\param[in]  front              Front direction.
\param[in]  up                 Up vector.
*/
glm::mat4 alignObject(glm::vec3& position, const glm::vec3& front, const glm::vec3& up)
{

	glm::vec3 z = -glm::normalize(front);

	if (isVectorNull(z))
		z = glm::vec3(0.0, 0.0, 1.0);

	glm::vec3 x = glm::normalize(glm::cross(up, z));

	if (isVectorNull(x))
		x = glm::vec3(1.0, 0.0, 0.0);

	glm::vec3 y = glm::cross(z, x);
	//mat4 matrix = mat4(1.0f);
	glm::mat4 matrix = glm::mat4(
		x.x, x.y, x.z, 0.0,
		y.x, y.y, y.z, 0.0,
		z.x, z.y, z.z, 0.0,
		position.x, position.y, position.z, 1.0);

	return matrix;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// CURVE SEGMENTS 
const size_t bat01CurveSize = 50;

///Control points of the animation curve for bat and ghost
glm::vec3 bat01CurveData[] = 
{
	glm::vec3(11.0, 4.0, 1.8),
	glm::vec3(10.0, 4.0, 1.8),
	glm::vec3(9.0, 4.0, 2.0),
	glm::vec3(8.0, 4.0, 1.9),
	glm::vec3(7.0, 4.0, 2.0),
	glm::vec3(6.0, 4.0, 2.0),
	glm::vec3(5.0, 4.0, 1.8),
	glm::vec3(4.0, 4.0, 1.9),
	glm::vec3(3.0, 4.0, 2.0),
	glm::vec3(2.0, 4.0, 2.0),
	glm::vec3(1.0, 4.0, 2.0),
	glm::vec3(0.0, 4.0, 1.9),
	glm::vec3(-1.0, 4.0, 2.0),
	glm::vec3(-2.0, 4.0, 2.0),
	glm::vec3(-3.0, 4.0, 1.8),
	glm::vec3(-4.0, 4.0, 1.9),
	glm::vec3(-5.0, 4.0, 1.8),
	glm::vec3(-6.0, 4.0, 2.0),
	glm::vec3(-7.0, 4.0, 2.0),
	glm::vec3(-8.0, 4.0, 1.9),
	glm::vec3(-9.0, 4.0, 1.8),
	glm::vec3(-10.0, 4.0, 2.0),
	glm::vec3(-11.0, 4.0, 1.8),
	glm::vec3(-12.0, 4.0, 1.9),
	glm::vec3(-13.0, 4.0, 2.0),
	glm::vec3(-12.0, 4.0, 2.0),
	glm::vec3(-11.0, 4.0, 2.0),
	glm::vec3(-10.0, 4.0, 1.9),
	glm::vec3(-9.0, 4.0, 2.0),
	glm::vec3(-8.0, 4.0, 2.0),
	glm::vec3(-7.0, 4.0, 1.8),
	glm::vec3(-6.0, 4.0, 1.9),
	glm::vec3(-5.0, 4.0, 2.0),
	glm::vec3(-4.0, 4.0, 1.8),
	glm::vec3(-3.0, 4.0, 2.0),
	glm::vec3(-2.0, 4.0, 1.9),
	glm::vec3(-1.0, 4.0, 2.0),
	glm::vec3(0.0, 4.0, 2.0),
	glm::vec3(1.0, 4.0, 2.0),
	glm::vec3(2.0, 4.0, 1.9),
	glm::vec3(3.0, 4.0, 1.8),
	glm::vec3(4.0, 4.0, 2.0),
	glm::vec3(5.0, 4.0, 2.0),
	glm::vec3(6.0, 4.0, 1.8),
	glm::vec3(7.0, 4.0, 2.0),
	glm::vec3(8.0, 4.0, 2.0),
	glm::vec3(9.0, 4.0, 2.0),
	glm::vec3(10.0, 4.0, 1.8),
	glm::vec3(11.0, 4.0, 2.0),
	glm::vec3(12.0, 4.0, 2.0),
	glm::vec3(13.0, 4.0, 2.0),
};

const size_t bat02CurveSize = 50;

glm::vec3 bat02CurveData[] = {

	glm::vec3(11.0, 3.7, 2.2),
	glm::vec3(10.0, 3.7, 2.2),
	glm::vec3(9.0, 3.7, 2.4),
	glm::vec3(8.0, 3.7, 2.3),
	glm::vec3(7.0, 3.7, 2.4),
	glm::vec3(6.0, 3.7, 2.4),
	glm::vec3(5.0, 3.7, 2.2),
	glm::vec3(4.0, 3.7, 2.3),
	glm::vec3(3.0, 3.7, 2.4),
	glm::vec3(2.0, 3.7, 2.4),
	glm::vec3(1.0, 3.7, 2.4),
	glm::vec3(0.0, 3.7, 2.3),
	glm::vec3(-1.0, 3.7, 2.4),
	glm::vec3(-2.0, 3.7, 2.4),
	glm::vec3(-3.0, 3.7, 2.4),
	glm::vec3(-4.0, 3.7, 2.3),
	glm::vec3(-5.0, 3.7, 2.2),
	glm::vec3(-6.0, 3.7, 2.1),
	glm::vec3(-7.0, 3.7, 2.0),
	glm::vec3(-8.0, 3.7, 1.9),
	glm::vec3(-9.0, 3.7, 1.8),
	glm::vec3(-10.0, 3.7, 2.0),
	glm::vec3(-11.0, 3.7, 1.8),
	glm::vec3(-12.0, 3.7, 1.9),
	glm::vec3(-13.0, 3.7, 2.0),
	glm::vec3(-12.0, 3.7, 2.1),
	glm::vec3(-11.0, 3.7, 2.2),
	glm::vec3(-10.0, 3.7, 2.2),
	glm::vec3(-9.0, 3.7, 2.2),
	glm::vec3(-8.0, 3.7, 2.1),
	glm::vec3(-7.0, 3.7, 1.9),
	glm::vec3(-6.0, 3.7, 1.9),
	glm::vec3(-5.0, 3.7, 2.0),
	glm::vec3(-4.0, 3.7, 1.9),
	glm::vec3(-3.0, 3.7, 1.9),
	glm::vec3(-2.0, 3.7, 1.9),
	glm::vec3(-1.0, 3.7, 1.9),
	glm::vec3(0.0, 3.7, 2.0),
	glm::vec3(1.0, 3.7, 2.1),
	glm::vec3(2.0, 3.7, 2.1),
	glm::vec3(3.0, 3.7, 2.1),
	glm::vec3(4.0, 3.7, 2.0),
	glm::vec3(5.0, 3.7, 2.0),
	glm::vec3(6.0, 3.7, 2.0),
	glm::vec3(7.0, 3.7, 1.9),
	glm::vec3(8.0, 3.7, 1.9),
	glm::vec3(9.0, 3.7, 1.9),
	glm::vec3(10.0, 3.7, 1.9),
	glm::vec3(11.0, 3.7, 2.0),
	glm::vec3(12.0, 3.7, 2.0),
	glm::vec3(13.0, 3.7, 2.0),
};

const size_t bat03CurveSize = 50;

glm::vec3 bat03CurveData[] = {
	
	glm::vec3(11.0, 4.0, 2.2),
	glm::vec3(10.0, 3.9, 2.2),
	glm::vec3(9.0, 3.8, 2.4),
	glm::vec3(8.0, 3.7, 2.3),
	glm::vec3(7.0, 3.6, 2.4),
	glm::vec3(6.0, 3.5, 2.4),
	glm::vec3(5.0, 3.4, 2.2),
	glm::vec3(4.0, 3.3, 2.3),
	glm::vec3(3.0, 3.2, 2.4),
	glm::vec3(2.0, 3.1, 2.4),
	glm::vec3(1.0, 3.0, 2.4),
	glm::vec3(0.0, 2.9, 2.3),
	glm::vec3(-1.0, 2.8, 2.4),
	glm::vec3(-2.0, 2.7, 2.4),
	glm::vec3(-3.0, 2.6, 2.4),
	glm::vec3(-4.0, 2.5, 2.3),
	glm::vec3(-5.0, 2.4, 2.2),
	glm::vec3(-6.0, 2.3, 2.1),
	glm::vec3(-7.0, 2.2, 2.0),
	glm::vec3(-8.0, 2.1, 1.9),
	glm::vec3(-9.0, 2.0, 1.8),
	glm::vec3(-10.0, 1.9, 2.0),
	glm::vec3(-11.0, 1.8, 1.8),
	glm::vec3(-12.0, 1.7, 1.9),
	glm::vec3(-13.0, 1.5, 2.0),
	glm::vec3(-12.0, 1.4, 2.1),
	glm::vec3(-11.0, 1.3, 2.2),
	glm::vec3(-10.0, 1.2, 2.2),
	glm::vec3(-9.0, 1.0, 2.2),
	glm::vec3(-8.0, 0.8, 2.1),
	glm::vec3(-7.0, 0.9, 1.9),
	glm::vec3(-6.0, 0.7, 1.9),
	glm::vec3(-5.0, 0.5, 2.0),
	glm::vec3(-4.0, 0.4, 1.9),
	glm::vec3(-3.0, 0.3, 1.9),
	glm::vec3(-2.0, 0.1, 1.9),
	glm::vec3(-1.0, 0.0, 1.9),
	glm::vec3(0.0, -0.2, 2.0),
	glm::vec3(1.0, -0.4, 2.1),
	glm::vec3(2.0, -0.6, 2.1),
	glm::vec3(3.0, -0.8, 2.1),
	glm::vec3(4.0, -0.9, 2.0),
	glm::vec3(5.0, -1.0, 2.0),
	glm::vec3(6.0, -1.2, 2.0),
	glm::vec3(7.0, -1.4, 1.9),
	glm::vec3(8.0, -1.7, 1.9),
	glm::vec3(9.0, -1.9, 1.9),
	glm::vec3(10.0, -2.0, 1.9),
	glm::vec3(11.0, -2.1, 2.0),
	glm::vec3(12.0, -2.2, 2.0),
	glm::vec3(13.0, -2.4, 2.0),
};

const size_t ghostCurveSize = 25;

glm::vec3 ghostCurveData[] = {

	glm::vec3((cos(0)*1.8), (sin(0)*1.8), 0.5),
	glm::vec3((cos(15)*1.8), (sin(15)*1.8), 0.2),
	glm::vec3((cos(30)*1.8), (sin(30)*1.8), 0.5),
	glm::vec3((cos(45)*1.8), (sin(45)*1.8), 0.2),
	glm::vec3((cos(60)*1.8), (sin(60)*1.8), 0.5),
	glm::vec3((cos(75)*1.8), (sin(75)*1.8), 0.2),
	glm::vec3((cos(90)*1.8), (sin(90)*1.8), 0.5),
	glm::vec3((cos(105)*1.8), (sin(105)*1.8), 0.2),
	glm::vec3((cos(120)*1.8), (sin(120)*1.8), 0.5),
	glm::vec3((cos(135)*1.8), (sin(135)*1.8), 0.2),

	glm::vec3((cos(150)*1.8), (sin(150)*1.8), 0.5),
	glm::vec3((cos(165)*1.8), (sin(165)*1.8), 0.2),
	glm::vec3((cos(180)*1.8), (sin(180)*1.8), 0.5),
	glm::vec3((cos(195)*1.8), (sin(195)*1.8), 0.2),
	glm::vec3((cos(210)*1.8), (sin(210)*1.8), 0.5),
	glm::vec3((cos(225)*1.8), (sin(225)*1.8), 0.2),
	glm::vec3((cos(240)*1.8), (sin(240)*1.8), 0.5),
	glm::vec3((cos(255)*1.8), (sin(255)*1.8), 0.2),
	glm::vec3((cos(270)*1.8), (sin(270)*1.8), 0.5),
	glm::vec3((cos(285)*1.8), (sin(285)*1.8), 0.2),

	glm::vec3((cos(300)*1.8), (sin(300)*1.8), 0.5),
	glm::vec3((cos(315)*1.8), (sin(315)*1.8), 0.2),
	glm::vec3((cos(330)*1.8), (sin(330)*1.8), 0.5),
	glm::vec3((cos(345)*1.8), (sin(345)*1.8), 0.2),
	glm::vec3((cos(360)*1.8), (sin(360)*1.8), 0.5),

};

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Evaluates a position on Catmull-Rom curve segment
glm::vec3 evaluateCurveSegment(
	glm::vec3& P0,
	glm::vec3& P1,
	glm::vec3& P2,
	glm::vec3& P3,
	const float t)
{
	glm::vec3 result(0.0, 0.0, 0.0);

	result = 0.5f * (P0 * (-1 * t*t*t + 2 * t*t - 1 * t) + P1 * (3 * t*t*t - 5 * t*t + 2.0f) + P2 * (-3 * t*t*t + 4 * t*t + t) + P3 * (t*t*t - t*t));
	return result;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Evaluates a first derivative of Catmull-Rom curve segment.
glm::vec3 evaluateCurveSegment_1stDerivative(
	glm::vec3& P0,
	glm::vec3& P1,
	glm::vec3& P2,
	glm::vec3& P3,
	const float t)
{
	glm::vec3 result(1.0, 0.0, 0.0);

	result = 0.5f * (P0 * (3 * (-1)*t*t + 2 * 2 * t - 1.0f) + P1 * (3 * 3 * t*t + 2 * (-5)*t) + P2 * (3 * (-3)*t*t + 2 * 4 * t + 1.0f) + P3 * (3 * t*t - 2 * t));
	return result;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Evaluates a position on a closed curve composed of Catmull-Rom segments.
glm::vec3 evaluateClosedCurve(
	glm::vec3 points[],
	const size_t count,
	const float t)
{
	glm::vec3 result(0.0, 0.0, 0.0);
	
	float x = cyclic_clamp(t, 0.0f, (float)count);
	size_t i = floor(x);
	result = evaluateCurveSegment(points[(i - 1 + count) % count], points[(i % count)], points[(i + 1) % count], points[(i + 2) % count], x - i);
	
	return result;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Evaluates a first derivative of a closed curve composed of Catmull-Rom segments.
glm::vec3 evaluateClosedCurve_1stDerivative(
	glm::vec3 points[],
	const size_t count,
	const float t)
{
	glm::vec3 result(1.0, 0.0, 0.0);

	float x = cyclic_clamp(t, 0.0f, (float)count);
	size_t i = floor(x);
	result = evaluateCurveSegment_1stDerivative(points[(i - 1 + count) % count], points[(i % count)], points[(i + 1) % count], points[(i + 2) % count], x - i);

	return result;
}