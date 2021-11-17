//----------------------------------------------------------------------------------------
/**
*      file		|		spline.h
*	   source	|		asteriods.cpp - spline.h
*/
//----------------------------------------------------------------------------------------
#ifndef __SPLINE_H
#define __SPLINE_H

#include "pgr.h"

/// Checks whether vector is zero-length or not.
bool isVectorNull(glm::vec3& vect);

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Align (rotate and move) current coordinate system to given parameters.
/**
This function work similarly to \ref gluLookAt, however it is used for object transform
rather than view transform. The current coordinate system is moved so that origin is moved
to the \a position. Object's local front (-Z) direction is rotated to the \a front and
object's local up (+Y) direction is rotated so that angle between its local up direction and
\a up vector is minimum.

\param[in]  position           Position of the origin.
\param[in]  front              Front direction.
\param[in]  up                 Up vector.
*/
glm::mat4 alignObject(glm::vec3& position, const glm::vec3& front, const glm::vec3& up);

// -----------------------------------------------------------------------------------------------------------------------------------------------------
///Control points of the animation curve for bat and ghost
extern glm::vec3 bat01CurveData[];
extern glm::vec3 bat02CurveData[];
extern glm::vec3 bat03CurveData[];

extern const size_t bat01CurveSize;
extern const size_t bat02CurveSize;
extern const size_t bat03CurveSize;

extern glm::vec3 ghostCurveData[];
extern const size_t ghostCurveSize;

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Cyclic clamping of a value.
/**
Makes sure that value is not outside the internal [\a minBound, \a maxBound].
If \a value is outside the interval it treated as periodic value with period equal to the size
of the interval. A necessary number of periods are added/subtracted to fit the value to the interval.

\param[in]  value              Value to be clamped.
\param[in]  minBound           Minimum bound of value.
\param[in]  maxBound           Maximum bound of value.
\return                        Value within range [minBound, maxBound].
\pre                           \a minBound is not greater that \maxBound.
*/
template <typename T>
T cyclic_clamp(const T value, const T minBound, const T maxBound)
{

	T amp = maxBound - minBound;
	T val = fmod(value - minBound, amp);

	if (val < T(0))
		val += amp;

	return val + minBound;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

/// Evaluates a position on Catmull-Rom curve segment.
glm::vec3 evaluateCurveSegment(
	glm::vec3& P0,
	glm::vec3& P1,
	glm::vec3& P2,
	glm::vec3& P3,
	const float t);
// -----------------------------------------------------------------------------------------------------------------------------------------------------

/// Evaluates a fist derivative of Catmull-Rom curve segment.
glm::vec3 evaluateCurveSegment_1stDerivative(
	glm::vec3& P0,
	glm::vec3& P1,
	glm::vec3& P2,
	glm::vec3& P3,
	const float t);
// -----------------------------------------------------------------------------------------------------------------------------------------------------

/// Evaluates a position on a closed curve composed of Catmull-Rom segments.
glm::vec3 evaluateClosedCurve(
	glm::vec3 points[],
	const size_t count,
	const float t);

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/// Evaluates a first derivative of a closed curve composed of Catmull-Rom segments.
glm::vec3 evaluateClosedCurve_1stDerivative(
	glm::vec3 points[],
	const size_t count,
	const float t);

#endif // __SPLINE_H
