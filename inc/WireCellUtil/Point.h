#ifndef WIRECELLUTIL_POINT_H
#define WIRECELLUTIL_POINT_H

#include "WireCellUtil/D3Vector.h"

#include <set>
#include <memory>		// auto_ptr

namespace WireCell {

    /// A 3D Cartesian point in double precision.

    typedef D3Vector<double> Point;

    /// An alias for Point.
    typedef Point Vector;

    /// A line segment running from a first (tail) to a second (head) point.
    typedef std::pair<Point, Point> Ray;

    /// PointVector - a collection of Points
    typedef std::vector<Point> PointVector;

    /// PointValue - an association of a point and a value
    typedef std::pair<Point, float> PointValue;

    /// PointValueVector - a collection of point-value associations
    typedef std::vector<PointValue> PointValueVector;

    /// Return true if lhs<rhs w/in tolerance.
    struct ComparePoints {
	bool operator()(const Point& lhs, const Point& rhs);
    };
    typedef std::set<Point, ComparePoints> PointSet;

    /// PointF - a 3D Cartesian point in single precision for when
    /// memory is constrained and double precision is not required.
    typedef D3Vector<float> PointF;


    
    /** Return true if point is contained in a rectangular solid
     * described by the ray bounds running between diagonally opposed
     * corners.*/
    bool point_contained(const Point& point, const Ray& bounds);

    /** Return true if point is contained by the bounding box along
     * the given axis (x=0, y=1, z=2) of the bounding box. */
    bool point_contained(const Point& point, const Ray& bounds, int axis);

    /** Return the angle from axis vector to vector. This is just
     * acos(dot).*/
    double point_angle(const Vector& axis, const Vector& vector);

    /** Return a ray representing the points of closest approach
     * between the two lines colinear with the two rays. */
    Ray ray_pitch(const Ray& ray1, const Ray& ray2);

    /** Return the distance from the tail to the head of the ray. */
    double ray_length(const Ray& ray);

    /** Return a vector going from ray's tail to ray's head. */
    Vector ray_vector(const Ray& ray);

    /** Return a unit vector pointing in the direction from the tail
     * to the head of the ray. */
    Vector ray_unit(const Ray& ray);

    /** Return the distance from the tail of the ray to the point
     * projected onto the ray's direction. */
    double ray_dist(const Ray& ray, const Point& point);

    struct BoundingBox {
	Ray bounds;

	/// Create a bounding box bounding an initial point.
	BoundingBox(const Point& initial);

	/// Create a bounding box bounding an initial ray.
	BoundingBox(const Ray& initial = Ray());

	/// Create a bounding box from an iterator pair.
	template<typename RayOrPointIterator>
	BoundingBox(const RayOrPointIterator& begin, const RayOrPointIterator& end) {
	    for (auto it = begin; it != end; ++it) { (*this)(*it); }
	}

	/// Enlarge bounds to hold point.
	void operator()(const Point& p);

	/// Enlarge bounds to hold ray.
	void operator()(const Ray& r);

	template<typename RayOrPointIterator>
	void operator()(const RayOrPointIterator& begin, const RayOrPointIterator& end) {
	    for (auto it = begin; it != end; ++it) { (*this)(*it); }
	}

	/// Return true if point is inside bounding box
	bool inside(const Point& point);

    };

}
std::ostream& operator<<(std::ostream& os, const WireCell::Ray& ray);



#endif

