#include "Triangle.h"
#include <iostream>
#include <cmath>

/**
 * Constructor for the Triangle class.
 * Initializes the triangle vertices, color, and normal vector.
 * @param v0 First vertex of the triangle.
 * @param v1 Second vertex of the triangle.
 * @param v2 Third vertex of the triangle.
 * @param color Color of the triangle
 */
Triangle::Triangle(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& color) : v0(v0), v1(v1), v2(v2), color(color) {
    // Normal = Normalize[(B - A) . (C - A)]
    normal = normalize(cross(v1 - v0, v2 - v0));
}

/**
 * Checks if a given ray intersects with the triangle.
 * Based on Möller–Trumbore intersection algorithm 
 * https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 * 
 * @param origin Origin of the ray.
 * @param direction Direction of the ray.
 * @param t Distance from the origin to the intersection point.
 * @param hitNormal Normal vector at the intersection point.
 * @return True if there is an intersection, false otherwise.
 */
bool Triangle::intersect(const vec3& origin, const vec3& direction, float& t, vec3& hitNormal) const {
    const float EPSILON = std::numeric_limits<float>::epsilon();

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 ray_cross_e2 = cross(direction, edge2);
    float det = dot(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON) {
        return false; // Ray is parallel to the triangle
    }

    float inv_det = 1.0f / det;
    vec3 s = origin - v0;
    float u = inv_det * dot(s, ray_cross_e2);
    
    if ((u < 0 && abs(u) > EPSILON) || (u > 1 && abs(u-1) > EPSILON)) {
        return false; // Intersection is outside the triangle
    }

    vec3 s_cross_e1 = cross(s, edge1);
    float v = inv_det * dot(direction, s_cross_e1);

    if ((v < 0 && abs(v) > EPSILON) || (u + v > 1 && abs(u + v - 1) > EPSILON)) {
        return false; // Intersection is outside the triangle
    }

    // compute t to find out where the intersection point is on the line.
    float t = inv_det * dot(edge2, s_cross_e1);

    if (t > EPSILON) { // Ray intersection
        hitNormal = normal;
        return true;
    } else {
        return false; // Line intersection but not ray intersection
    }
}

/**
 * Returns the center of the triangle.
 * @return Center of the triangle.
 */
vec3 Triangle::getCenter() const {
    vec3 center;
    center.x = (v0.x + v1.x + v2.x)/3;
    center.y = (v0.y + v1.y + v2.y)/3;
    center.z = (v0.z + v1.z + v2.z)/3;
    return center;
}

/**
 * Checks if the triangle is within a given bounding box.
 * If a given vertex of the triangle is inside the box, it returns true.
 * @param min Minimum corner of the bounding box.
 * @param max Maximum corner of the bounding box.
 * @return True if the triangle is inside the box, false otherwise.
 */
bool Triangle::isInBox(const vec3& min, const vec3& max) const {
    return (v0.x >= min.x && v0.x <= max.x && v0.y >= min.y && v0.y <= max.y && v0.z >= min.z && v0.z <= max.z) ||
           (v1.x >= min.x && v1.x <= max.x && v1.y >= min.y && v1.y <= max.y && v1.z >= min.z && v1.z <= max.z) ||
           (v2.x >= min.x && v2.x <= max.x && v2.y >= min.y && v2.y <= max.y && v2.z >= min.z && v2.z <= max.z);
}