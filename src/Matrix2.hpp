#pragma once

#include <cuda.h>
#include <iostream>
#include <cstdio>

namespace spann
{
  template<typename domain>
  class Matrix
  {
    public:
      int rows, columns;

      domain* data = 0;
      domain* tmp  = 0;

      Matrix() {}

      ~Matrix()
      {
        delete[] data;
        delete[] tmp;
      }

      Matrix( int rows, int columns, domain value = 0 )
      {
        SetSize( rows, columns, value );
      }

      void SetSize( int rows, int columns, domain value = 0 )
      {
        this->rows    = rows;
        this->columns = columns;

        data = new domain[rows * columns];
        tmp  = new domain[rows * columns];

        for(int i = 0; i < rows * columns; ++i)
        {
          data[i] = value;
          tmp[i]  = value;
        }
      }

      Matrix( const Matrix<domain>& other )
      {
        data = new domain[other.rows * other.columns];
        tmp  = new domain[other.rows * other.columns];
        std::copy( other.data, other.data + (other.rows * other.columns), data );
        std::copy( other.tmp , other.tmp  + (other.rows * other.columns), tmp );

        rows    = other.rows;
        columns = other.columns;
      }

      domain* operator[]( int row ) const { return data + (row * columns); }

      __host__ __device__
      domain* operator()( int index ) const { return data + index; }

      Matrix<domain>& operator+=( const Matrix<domain>& b )
      {
        for( int i = 0; i < rows * columns; ++i )
          data[i] += *b(i);
        return *this;
      }

      __device__
      void Add( const Matrix<domain>& b )
      {
        for( int i = 0; i < rows * columns; ++i )
        {
          data[i] += 100;//*b(i);
          printf("Call for value :\n");
        }
      }

      __device__
      void Add( const Matrix<domain>* b )
      {
        for( int i = 0; i < rows * columns; ++i )
        {
          data[i] += 100;//*(*b)(i);
          printf("Call for value :\n");
        }
      }

      Matrix<domain> operator+( const Matrix<domain>& b )
      {
        Matrix<domain> result = *this;
        result += b;
        return result;
      }

      Matrix<domain>& operator-=( const Matrix<domain>& b )
      {
        for( int i = 0; i < rows * columns; ++i )
          data[i] -= *b(i);
        return *this;
      }

      Matrix<domain> operator-( const Matrix<domain>& b )
      {
        Matrix<domain> result = *this;
        result -= b;
        return result;
      }

      Matrix<domain>& operator*=( const Matrix<domain>& b )
      {
        *this = *this * b;
        return *this;
      }

      Matrix<domain>& operator*=( const domain& a )
      {
        *this = *this * a;
        return *this;
      }

      Matrix<domain> operator*( const Matrix<domain>& b )
      {
        Matrix<domain> result( rows, b.columns );

        for( int i = 0; i < rows; ++i )
          for( int j = 0; j < b.columns; ++j )
            for( int k = 0; k < columns; ++k )
              result[i][j] += (data + (i * columns))[k] * b[k][j];

        return result;
      }

      Matrix<domain> operator*( const domain& a )
      {
        Matrix<domain> result( rows, columns );

        for( int i = 0; i < rows; ++i )
            for( int j = 0; j < columns; ++j )
              result[i][j] += (data + (i * columns))[j] * a;

        return result;
      }

      // Use only when dim(A*B) = dim(A)
      void Multiplication( const Matrix<domain>& b )
      {
        domain* swap;
        for(int i = 0; i < rows; ++i)
          for(int j = 0; j < b.columns; ++j)
            for(int k = 0; k < columns; ++k)
              tmp[i * columns + j] += data[i * columns + k] * b[k][j];
        swap = data;
        data = tmp;
        tmp  = swap;
        for(int i = 0; i < rows * columns; ++i)
          tmp[i] = 0;
      }

      void HadamardProduct( const Matrix<domain>& b )
      {
        for( int i = 0; i < rows * columns; ++i )
          data[i] *= *b(i);
      }

      void Multiplication( const Matrix<domain>& a, const Matrix<domain>& b )
      {
        for( int i = 0; i < a.rows; ++i )
          for( int j = 0; j < b.columns; ++j )
            for( int k = 0; k < a.columns; ++k )
              data[i * b.columns + j] += a[i][k] * b[k][j];
      }

      Matrix<domain> operator-() const
      {
        Matrix<domain> result( rows, columns );
        for( int i = 0; i < rows * columns; ++i )
          *result(i) = -data[i];
        return result;
      }

      Matrix<domain> operator!() const
      {
        Matrix<domain> result( columns, rows );
        for( int i = 0; i < columns; ++i )
          for( int j = 0; j < rows; ++j )
            result[i][j] = (data + (j * columns))[i];
        return result;
      }

      Matrix<domain>& operator=( const Matrix<domain>& other )
      {
        delete[] data;
        delete[] tmp;

        data = new domain[other.rows * other.columns];
        tmp  = new domain[other.rows * other.columns];
        std::copy( other.data, other.data + (other.rows * other.columns), data );
        std::copy( other.tmp , other.tmp  + (other.rows * other.columns), tmp );

        rows    = other.rows;
        columns = other.columns;

        return *this;
      }

      Matrix<domain>& operator=( const domain& other )
      {
        for(int i = 0; i < rows * columns; ++i)
          data[i] = other;

        return *this;
      }

      void Copy( const Matrix<domain>& other )
      {
        columns = other.columns;
        rows    = other.rows;

        for( int i = 0; i < rows * columns; ++i )
          data[i] = *other( i );
      }

      void operator()( const Matrix<domain>& other )
      {
        std::copy( other.data, other.data + (other.rows * other.columns), data );
      }

      Matrix<domain>& operator()( domain (*f)( domain ) )
      {
        for(int i = 0; i < rows * columns; ++i)
          data[i] = f( data[i] );

        return *this;
      }

      Matrix<domain> Apply( domain (*f)( domain ) )
      {
        Matrix<domain> result = *this;
        result(f);

        return result;
      }

      void Initialize( int rows, int columns )
      {
        data = new domain[rows * columns];
        tmp  = new domain[rows * columns];

        this->rows    = rows;
        this->columns = columns;
      }

      domain SumElements()
      {
        domain result = 0;
        for( int i = 0; i < rows * columns; ++i )
          result += data[i];
        return result;
      }

      friend std::ostream& operator<< (std::ostream& stream, const Matrix<domain>& m)
      {
        for( int j = 0; j < m.rows; ++j )
        {
          for( int k = 0; k < m.columns; ++k )
            stream << m.data[ j * m.columns + k ] << '|';
          stream << '\n';
        }
        return stream;
      }
  };
}
