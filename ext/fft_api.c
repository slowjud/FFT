#include "ruby.h"
#include <math.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif



// Validate the incoming array
static int validate( VALUE self ) {
    long length;
    VALUE *values = RARRAY_PTR( self );

    // Check we're an Array
    Check_Type( self, T_ARRAY );

    // make sure we have an array of two arrays
    if( RARRAY_LEN( self ) != 2 ) return FALSE;
    
    // make sure the arrays are of the same length
    if( RARRAY_LEN( values[0] ) != RARRAY_LEN( values[1] ) ) return FALSE;

    // make sure the size of the array is a power of 2
    length = RARRAY_LEN( values[0] );
    if( ( length < 2 ) || ( length & ( length - 1 ) ) ) return FALSE;

    return TRUE;
}

static VALUE perform_fft( double **values, long length, int direction ){
    VALUE outArray = rb_ary_new2( 2 );
    VALUE xArray = rb_ary_new2( length );
    VALUE yArray = rb_ary_new2( length );
    
    long exponent, i, i1, j, k, i2, l, l1, l2;
    double c1,c2,tx,ty,t1,t2,u1,u2,z;

    // Caculate the exponent
    exponent = 0;
    i = length;
    while( i >>= 1 ){
        exponent++;
    }

    // Do the bit reversal
    i2 = length >> 1;
    j = 0;
    for( i = 0; i < ( length - 1 ); i++ ){
        if( i < j ){
            tx = values[0][i];
            ty = values[1][i];
            values[0][i] = values[0][j];
            values[1][i] = values[1][j];
            values[0][j] = tx;
            values[1][j] = ty;
        }
        k = i2;
        while( k <= j ){
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    
    // Compute the FFT
    c1 = -1.0;
    c2 = 0.0;
    l2 = 1;
    for( l = 0; l < exponent; l++ ){
        l1 = l2;
        l2 <<= 1;
        u1 = 1.0;
        u2 = 0.0;
        for( j = 0; j < l1; j++ ){
            for( i = j; i < length; i += l2){
                i1 = i + l1;
                t1 = u1 * values[0][i1] - u2 * values[1][i1];
                t2 = u1 * values[1][i1] + u2 * values[0][i1];
                values[0][i1] = values[0][i] - t1;
                values[1][i1] = values[1][i] - t2;
                values[0][i] += t1;
                values[1][i] += t2;
            }
            z = u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = sqrt( ( 1.0 - c1 ) / 2.0 );
        if( direction == 1 ){
            c2 = -c2;
        }
        c1 = sqrt( ( 1.0 + c1 ) / 2.0 );
    }

    // Scaling for forward transform
    if( direction == 1 ){
        for( i = 0; i < length; i++ ){
            values[0][i] /= length;
            values[1][i] /= length;
        }
    }
    // convert the doubles into ruby numbers and stick them into a ruby array
    for( i = 0; i < length; i++ ){
        rb_ary_push( xArray, DBL2NUM( values[0][i] ) );
        rb_ary_push( yArray, DBL2NUM( values[1][i] ) );
    }

    rb_ary_push( outArray, xArray );
    rb_ary_push( outArray, yArray );
    
    return outArray;
}

// perform the fft
static VALUE prepare_fft( VALUE inArray, int direction ){
    long i, length;
    VALUE *values;
    double **transformed;
    VALUE outArray;

    if( !validate( inArray ) ) {
        return Qnil;
    }

    // convert the ruby array into a C array of integers using NUM2DBL(Fixnum)
    values = RARRAY_PTR( inArray );
    length = RARRAY_LEN( values[0] );
    transformed = malloc( sizeof( double ) * 2 );
    transformed[0] = malloc( sizeof( double* ) * RARRAY_LEN( values[0] ) );
    transformed[1] = malloc( sizeof( double* ) * RARRAY_LEN( values[1] ) );
    for( i = 0; i < length; i++ ) {
        // process values
        transformed[0][i] = NUM2DBL( RARRAY_PTR( values[0] )[i] );
        transformed[1][i] = NUM2DBL( RARRAY_PTR( values[1] )[i] );
    }
    
    // do the actual transform
    outArray = perform_fft( transformed, length, direction );

    // No memory leaks here!
    free( transformed[0] );
    free( transformed[1] );
    free( transformed );
    
    return outArray;
}

static VALUE forward_fft( VALUE self ){
    return prepare_fft( self, 1 );
}

static VALUE reverse_fft( VALUE self ){
    return prepare_fft( self, -1 );
}

// initalizer
VALUE FFT;
void Init_fft_api() {
    FFT = rb_define_module("FFTAPI");
    rb_define_method(FFT, "fft", forward_fft, 0);
    rb_define_method(FFT, "rfft", reverse_fft, 0);
}