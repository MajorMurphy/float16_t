//
// License: BSD, Anti-996
//
// inspired by:
// https://github.com/acgessler/half_float
// https://github.com/x448/float16
//
#include <cstdint>
#include <limits>
#include <iostream>
#include <cmath>
#include <bitset>

namespace numeric
{
    constexpr unsigned long const version = 1UL;
    #ifdef DEBUG
    constexpr unsigned long const debug_mode = 1;
    #else
    constexpr unsigned long const debug_mode = 0;
    #endif

    namespace float16_t_private
    {

        union float16
        {
            std::uint16_t bits_;
            struct
            {
                std::uint16_t frac_ : 10;
                std::uint16_t exp_  : 5;
                std::uint16_t sign_ : 1;
            } ieee_;
        };

        template< std::integral T >
        constexpr float16 to_float16( T val ) noexcept
        {
            return float16{ static_cast<std::uint16_t>( val ) };
        }

        inline std::ostream& operator<<( std::ostream& os, float16 const& f16 )
        {
            os << std::bitset<1>( f16.ieee_.sign_ ) << " "
               << std::bitset<5>( f16.ieee_.exp_ ) << " "
               << std::bitset<10>( f16.ieee_.frac_ );
            return os;
        }

        union float32
        {
            std::uint32_t bits_;
            struct
            {
                uint32_t frac_ : 23;
                uint32_t exp_  : 8;
                uint32_t sign_ : 1;
            } ieee_;
            float float_;
        };

        inline std::ostream& operator<<( std::ostream& os, float32 const& f32 )
        {
            os << std::bitset<1>( f32.ieee_.sign_ ) << " "
               << std::bitset<8>( f32.ieee_.exp_ ) << " "
               << std::bitset<23>( f32.ieee_.frac_ ) << " ("
               << f32.float_ << ")";
            return os;
        }

        inline constexpr float16 float32_to_float16( float input ) noexcept
        {
            using float16_t_private::to_float16;
            float32 f32;
            f32.float_ = input;
            std::uint32_t const u32 = f32.bits_;
            std::uint32_t sign = u32 & 0x80000000;
            std::uint32_t exponent = u32 & 0x7f800000;
            std::uint32_t coef = u32 & 0x7fffff;

            if ( exponent == 0x7f800000 )
            {
                std::uint32_t nan_bit = 0;

                if ( coef != 0 )
                    nan_bit = 0x200;

                return to_float16( ( sign >> 16 ) | 0x7c00 | nan_bit | ( coef >> 13 ) );
            }

            std::uint32_t half_sign = sign >> 16;
            std::int32_t unbiased_exponent = static_cast<std::uint32_t>( exponent >> 23 ) - 127;
            std::int32_t half_exponent = unbiased_exponent + 15;

            if ( half_exponent == 0x1f )
                return to_float16( half_sign | 0x7c00 );

            if ( half_exponent <= 0 )
            {
                if ( half_exponent < -10 )
                    return to_float16( half_sign  );

                std::uint32_t c = coef | 0x800000;
                std::uint32_t half_coef = c >> ( 14 - half_exponent );
                std::uint32_t round_bit = 1 << ( 13 - half_exponent );

                if ( ( c & round_bit ) && ( c & ( 3 * round_bit - 1 ) ) )
                    ++half_coef;

                return to_float16( half_sign | half_coef );
            }

            std::uint32_t uhalf_exponent = half_exponent << 10;
            std::uint32_t half_coef = coef >> 13;
            std::uint32_t round_bit = 0x1000;

            if ( ( coef & round_bit ) && ( coef & ( 3 * round_bit - 1 ) ) )
                return to_float16( ( half_sign | uhalf_exponent | half_coef ) + 1 );

            return to_float16( ( half_sign | uhalf_exponent | half_coef ) );
        }

        inline constexpr float32 float16_to_float32( std::uint16_t input ) noexcept
        {
            std::uint32_t sign = ( input & 0x8000 ) << 16;
            std::uint32_t exponent =  ( input & 0x7c00 ) >> 10;
            std::uint32_t coef = ( input & 0x3ff ) << 13;

            if ( exponent == 0x1f )
            {
                if ( coef == 0 )
                    return float32{ sign | 0x7f800000 | coef };

                return float32{ sign | 0x7fc00000 | coef };
            }

            if ( exponent == 0 )
            {
                if ( coef == 0 )
                    return float32{ sign };

                ++exponent;

                while( coef & 0x7f800000 )
                {
                    coef <<= 1;
                    --exponent;
                }

                coef &= 0x7fffff;
            }

            return float32{ sign | ( exponent + 0x70 ) << 23 | coef };
        }

    }//namespace float16_private

    struct float16_t
    {
        float16_t_private::float16 data_;

        constexpr float16_t() noexcept = default;
        constexpr float16_t( float16_t const& ) noexcept = default;
        constexpr float16_t( float16_t&& ) noexcept = default;
        constexpr float16_t( float other ) noexcept : data_ { float16_t_private::float32_to_float16( other ) } { }
        constexpr float16_t( std::uint16_t bits ) noexcept : data_{ bits } { }

        constexpr float16_t& operator = ( float16_t const& ) noexcept = default;
        constexpr float16_t& operator = ( float16_t&& ) noexcept = default;
        constexpr float16_t& operator = ( std::uint16_t bits ) noexcept
        {
            data_.bits_ = bits;
            return *this;
        }
        constexpr float16_t& operator = ( float other ) noexcept
        {
            data_ = float16_t_private::float32_to_float16( other );
            return *this;
        }

        constexpr operator float() const noexcept
        {
            auto f32 = float16_t_private::float16_to_float32( data_.bits_ );
            return f32.float_;
        }

        constexpr operator std::uint16_t() const noexcept
        {
            return data_.bits_;
        }

        constexpr float16_t& operator += ( float16_t v ) noexcept
        {
            *this = float(*this) + float(v);
            return *this;
        }

        constexpr float16_t& operator -= ( float16_t v ) noexcept
        {
            *this = float(*this) - float(v);
            return *this;
        }

        constexpr float16_t& operator *= ( float16_t v ) noexcept
        {
            *this = float(*this) * float(v);
            return *this;
        }

        constexpr float16_t& operator /= ( float16_t v ) noexcept
        {
            *this = float(*this) / float(v);
            return *this;
        }

        constexpr float16_t& operator += ( float v ) noexcept
        {
            *this = float(*this) + v;
            return *this;
        }

        constexpr float16_t& operator -= ( float v ) noexcept
        {
            *this = float(*this) - v;
            return *this;
        }

        constexpr float16_t& operator *= ( float v ) noexcept
        {
            *this = float(*this) * v;
            return *this;
        }

        constexpr float16_t& operator /= ( float v ) noexcept
        {
            *this = float(*this) / v;
            return *this;
        }

        constexpr float16_t operator -- () noexcept //--f
        {
            *this -= 1.0f;
            return *this;
        }

        constexpr float16_t operator -- (int) noexcept // f--
        {
            float16_t ans{*this};
            *this -= 1.0f;
            return ans;
        }

        constexpr float16_t operator ++ () noexcept //++f
        {
            *this += 1.0f;
            return *this;
        }

        constexpr float16_t operator ++ (int) noexcept // f++
        {
            float16_t ans{*this};
            *this += 1.0f;
            return ans;
        }

        constexpr float16_t operator - () const noexcept
        {
            return float16_t{ static_cast<std::uint16_t>((data_.bits_ & 0x7fff) | (data_.bits_ ^ 0x8000 )) };
        }

        constexpr float16_t operator + () const noexcept
        {
            return *this;
        }

    }; //struct float16_t

    constexpr float16_t         fp16_infinity{ static_cast<std::uint16_t>(0x7c00) };
    constexpr float16_t         fp16_max{ static_cast<std::uint16_t>(0x7bff) }; //65504
    constexpr float16_t         fp16_max_subnormal{ static_cast<std::uint16_t>(0x3ff) }; // 0.00006097
    constexpr float16_t         fp16_min{ static_cast<std::uint16_t>(0xfbff) };
    constexpr float16_t         fp16_min_positive{ static_cast<std::uint16_t>(0x400) };
    constexpr float16_t         fp16_min_positive_subnormal{ static_cast<std::uint16_t>(0x1) };
    constexpr float16_t         fp16_nan{ static_cast<std::uint16_t>(0x7e00) };
    constexpr float16_t         fp16_infinity_negative{ static_cast<std::uint16_t>(0xfc00) };

    constexpr float16_t         fp16_one{ static_cast<std::uint16_t>(0x3c00) };
    constexpr float16_t         fp16_zero{ static_cast<std::uint16_t>(0x0) };
    constexpr float16_t         fp16_zero_negative{ static_cast<std::uint16_t>(0x8000) };
    constexpr float16_t         fp16_e{ static_cast<std::uint16_t>(0x4170) };
    constexpr float16_t         fp16_pi{ static_cast<std::uint16_t>(0x4248) };

    constexpr float16_t operator + ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) + float(rhs);
    }

    constexpr float16_t operator - ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) - float(rhs);
    }

    constexpr float16_t operator * ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) * float(rhs);
    }

    constexpr float16_t operator / ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) / float(rhs);
    }

    constexpr bool operator < ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) < float(rhs);
    }

    constexpr bool operator <= ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) <= float(rhs);
    }

    constexpr bool operator == ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) == float(rhs);
    }

    constexpr bool operator > ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) > float(rhs);
    }

    constexpr bool operator >= ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) >= float(rhs);
    }

    constexpr bool operator != ( float16_t lhs, float16_t rhs ) noexcept
    {
        return float(lhs) != float(rhs);
    }

    template<typename CharT, class Traits>
    std::basic_ostream<CharT, Traits>& operator << ( std::basic_ostream<CharT, Traits>& os, float16_t const& f )
    {
        std::basic_ostringstream<CharT, Traits> __s;
        __s.flags(os.flags());
        __s.imbue(os.getloc());
        __s.precision(os.precision());

        __s << float(f);
        if constexpr( debug_mode )
        {
            __s << "(";
            __s  << std::bitset<1>( f.data_.ieee_.sign_ ) << " ";
            __s  << std::bitset<5>( f.data_.ieee_.exp_ ) << " ";
            __s  << std::bitset<10>( f.data_.ieee_.frac_ ) << ")";
        }

        return os << __s.str();
    }

    template<typename CharT, class Traits>
    std::basic_istream<CharT, Traits>& operator >> ( std::basic_istream<CharT, Traits>& is, float16_t& f )
    {
        bool __fail = true;
        float __v;

        if ( is >> __v )
        {
            __fail = false;
            f = __v;
        }

        if (__fail)
            is.setstate(std::ios_base::failbit);

        return is;
    }


    //TODO: all functions in <cmath>

    constexpr float16_t abs( float16_t f ) noexcept
    {
        float16_t ans{f};
        ans.data_.bits_ &= 0x7fff;
        return ans;
    }

    namespace float16_t_private
    {
        auto constexpr make_unary_function = []( auto const& func ) noexcept
        {
            return [func]( float16_t f ) -> float16_t { return func( float(f) ); };
        };

        auto constexpr make_binary_function = []( auto const& func ) noexcept
        {
            return [func]( float16_t f1, float16_t f2 ) -> float16_t { return func( float(f1), float(f2) ); };
        };

        auto constexpr make_trinary_function = []( auto const& func ) noexcept
        {
            return [func]( float16_t f1, float16_t f2, float16_t f3 ) -> float16_t { return func( float(f1), float(f2), float(f3) ); };
        };

    }//float_t_private

    constexpr auto fmod = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::fmod( f1, f2 ); } );
    constexpr auto remainder = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::remainder( f1, f2 ); } );
    //remquo ??
    constexpr auto fma = float16_t_private::make_trinary_function( []( float f1, float f2, float f3 ) { return std::fma( f1, f2, f3 ); } );
    constexpr auto fmax = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::fmax( f1, f2 ); } );
    constexpr auto fmin = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::fmax( f1, f2 ); } );
    constexpr auto fdim = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::fdim( f1, f2 ); } );
    constexpr auto lerp = float16_t_private::make_trinary_function( []( float f1, float f2, float f3 ) { return std::lerp( f1, f2, f3 ); } );
    constexpr auto exp = float16_t_private::make_unary_function( [](float f){ return std::exp(f); } );
    constexpr auto exp2 = float16_t_private::make_unary_function( [](float f){ return std::exp2(f); } );
    constexpr auto expm1 = float16_t_private::make_unary_function( [](float f){ return std::expm1(f); } );
    constexpr auto log = float16_t_private::make_unary_function( [](float f){ return std::log(f); } );
    constexpr auto log10 = float16_t_private::make_unary_function( [](float f){ return std::log10(f); } );
    constexpr auto log2 = float16_t_private::make_unary_function( [](float f){ return std::log2(f); } );
    constexpr auto log1p = float16_t_private::make_unary_function( [](float f){ return std::log1p(f); } );
    constexpr auto pow = float16_t_private::make_binary_function( []( float f1, float f2 ) { return std::pow( f1, f2 ); } );
    constexpr auto sqrt = float16_t_private::make_unary_function( [](float f){ return std::sqrt(f); } );
    constexpr auto cbrt = float16_t_private::make_unary_function( [](float f){ return std::cbrt(f); } );

    inline float16_t hypot( float16_t f1, float16_t f2 )
    {
        return std::hypot( float(f1), float(f2) );
    }

    /* -- Should be valid after c++17 for 3 parameter hypot
    inline float16_t hypot( float16_t f1, float16_t f2, float16_t )
    {
        return std::hypot( float(f1), float(f2), float(f3) );
    }
    */

    constexpr auto sin = float16_t_private::make_unary_function( [](float f){ return std::sin(f); } );
    constexpr auto sinh = float16_t_private::make_unary_function( [](float f){ return std::sinh(f); } );
    constexpr auto cos = float16_t_private::make_unary_function( [](float f){ return std::cos(f); } );
    constexpr auto cosh = float16_t_private::make_unary_function( [](float f){ return std::cosh(f); } );
    constexpr auto tan = float16_t_private::make_unary_function( [](float f){ return std::tan(f); } );
    constexpr auto tanh = float16_t_private::make_unary_function( [](float f){ return std::tanh(f); } );
    constexpr auto asin = float16_t_private::make_unary_function( [](float f){ return std::asin(f); } );
    constexpr auto asinh = float16_t_private::make_unary_function( [](float f){ return std::asinh(f); } );
    constexpr auto acos = float16_t_private::make_unary_function( [](float f){ return std::acos(f); } );
    constexpr auto acosh = float16_t_private::make_unary_function( [](float f){ return std::acosh(f); } );
    constexpr auto atan = float16_t_private::make_unary_function( [](float f){ return std::atan(f); } );
    constexpr auto atanh = float16_t_private::make_unary_function( [](float f){ return std::atanh(f); } );
    constexpr auto atan2 = float16_t_private::make_binary_function( [](float f1, float f2){ return std::atan2(f1, f2); } );

    constexpr auto erf = float16_t_private::make_unary_function( [](float f){ return std::erf(f); } );
    constexpr auto erfc = float16_t_private::make_unary_function( [](float f){ return std::erfc(f); } );
    constexpr auto tgamma = float16_t_private::make_unary_function( [](float f){ return std::tgamma(f); } );
    constexpr auto lgamma = float16_t_private::make_unary_function( [](float f){ return std::lgamma(f); } );
    constexpr auto ceil = float16_t_private::make_unary_function( [](float f){ return std::ceil(f); } );
    constexpr auto floor = float16_t_private::make_unary_function( [](float f){ return std::floor(f); } );
    constexpr auto trunc = float16_t_private::make_unary_function( [](float f){ return std::trunc(f); } );
    constexpr auto round = float16_t_private::make_unary_function( [](float f){ return std::round(f); } );
    constexpr auto nearbyint = float16_t_private::make_unary_function( [](float f){ return std::nearbyint(f); } );
    constexpr auto rint = float16_t_private::make_unary_function( [](float f){ return std::rint(f); } );

    //Floating point manipulation functions not defined
    //frexp ldexp, modf, scalbn, ilogb

    constexpr auto logb = float16_t_private::make_unary_function( [](float f){ return std::logb(f); } );
    constexpr auto nextafter = float16_t_private::make_binary_function( [](float f1, float f2){ return std::nextafter(f1, f2); } );
    constexpr auto copysign = float16_t_private::make_binary_function( [](float f1, float f2){ return std::copysign(f1, f2); } );

    constexpr bool is_nan( float16_t f16 ) noexcept
    {
        return (std::uint16_t(f16) & 0x7fff) > 0x7f80;
    }

    constexpr bool is_inf( float16_t f16 ) noexcept
    {
        return (std::uint16_t(f16) & 0x7fff) == 0x7f80;
    }

    constexpr bool is_finite( float16_t f16 ) noexcept
    {
        return (std::uint16_t(f16) & 0x7f80) != 0x7f80;
    }

    constexpr bool is_normal( float16_t f16 ) noexcept
    {
        auto const exponent = std::uint16_t(f16) & 0x7f80;
        return (exponent != 0x7f80) && (exponent != 0);
    }

    constexpr bool is_positive( float16_t f16 ) noexcept
    {
        return ((std::uint16_t(f16)) & 0x8000) == 0;
    }

    constexpr bool is_negative( float16_t f16 ) noexcept
    {
        return (std::uint16_t(f16)) & 0x8000;
    }

    //special functions not defined
    // assoc_laguerre, asso_legendre, hermite, legendre, laguerre, sph_bessel, sph_legendre, sph_neumann
    //

    constexpr auto beta = float16_t_private::make_binary_function( [](float f1, float f2){ return std::beta(f1, f2); } );
    constexpr auto comp_ellint_1 = float16_t_private::make_unary_function( [](float f){ return std::comp_ellint_1(f); } );
    constexpr auto comp_ellint_2 = float16_t_private::make_unary_function( [](float f){ return std::comp_ellint_2(f); } );
    constexpr auto comp_ellint_3 = float16_t_private::make_binary_function( [](float f1, float f2){ return std::comp_ellint_3(f1, f2); } );
    constexpr auto cyl_bessel_i = float16_t_private::make_binary_function( [](float f1, float f2){ return std::cyl_bessel_i(f1, f2); } );
    constexpr auto cyl_bessel_j = float16_t_private::make_binary_function( [](float f1, float f2){ return std::cyl_bessel_j(f1, f2); } );
    constexpr auto cyl_bessel_k = float16_t_private::make_binary_function( [](float f1, float f2){ return std::cyl_bessel_k(f1, f2); } );
    constexpr auto cyl_neumann = float16_t_private::make_binary_function( [](float f1, float f2){ return std::cyl_neumann(f1, f2); } );
    constexpr auto ellint_1 = float16_t_private::make_binary_function( [](float f1, float f2){ return std::ellint_1(f1, f2); } );
    constexpr auto ellint_2 = float16_t_private::make_binary_function( [](float f1, float f2){ return std::ellint_2(f1, f2); } );
    constexpr auto ellint_3 = float16_t_private::make_trinary_function( [](float f1, float f2, float f3){ return std::ellint_3(f1, f2, f3); } );
    constexpr auto expint = float16_t_private::make_unary_function( [](float f){ return std::expint(f); } );
    constexpr auto riemann_zeta = float16_t_private::make_unary_function( [](float f){ return std::riemann_zeta(f); } );


}//namespace numeric

