#ifndef VECUTILS_H
#define VECUTILS_H

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <cassert>

namespace vecutils
{

template <typename T>
T dotProduct(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <typename T>
sf::Vector3<T> crossProduct(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs)
{
    return sf::Vector3<T>(
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x);
}

template <typename T>
T lengthSquared(const sf::Vector3<T>& vector)
{
    return dotProduct(vector, vector);
}

template <typename T>
T length(const sf::Vector3<T>& vector)
{
    return sqrt(lengthSquared(vector));
}

template <typename T>
T distance(const sf::Vector3<T>& pos1, const sf::Vector3<T>& pos2)
{
    return sqrt(distanceSquared(pos1, pos2));
}

template <typename T>
T distanceSquared(const sf::Vector3<T>& pos1, const sf::Vector3<T>& pos2)
{
    return pow(pos1.x-pos2.x, 2) + pow(pos1.y-pos2.y, 2) + pow(pos1.z-pos2.z, 2);
}

template <typename T>
void normalize(sf::Vector3<T>& vector)
{
    T len = length(vector);
    vector /= len;
}

} // namespace vecutils

#endif // VECUTILS_H
