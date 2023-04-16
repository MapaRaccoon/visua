#pragma once

#include <concepts>
#include <iostream>

template < typename T >
requires std::floating_point< T > && std::default_initializable< T >
struct Stereo
{
    Stereo( T left, T right ) : left( left ), right( right ) { }
    Stereo() : left( T{} ), right( T{} ) { }
    T left;
    T right;
};

template < typename T >
Stereo< T > operator+( const Stereo< T > a, const Stereo< T > b )
{
    return { a.left + b.left, a.right + b.right };
}

template < typename T >
Stereo< T > operator-( const Stereo< T > a, const Stereo< T > b )
{
    return { a.left - b.left, a.right - b.right };
}

template < typename T >
Stereo< T > operator*( const T c, const Stereo< T > a )
{
    return { c * a.left, c * a.right };
}

template < typename T >
Stereo< T > operator*( const Stereo< T > a, const T c )
{
    return { c * a.left, c * a.right };
}

template < typename T >
Stereo< T > operator+( const T c, const Stereo< T > a )
{
    return { c + a.left, c + a.right };
}

template < typename T >
Stereo< T > operator+( const Stereo< T > a, const T c )
{
    return { c + a.left, c + a.right };
}

template < typename T >
requires requires(std::ostream os, T t) {
    os << t;
}
std::ostream &operator<<(std::ostream &os, const Stereo<T> ts) {
    return os << "(" << ts.left << ", " << ts.right << ")";
}

