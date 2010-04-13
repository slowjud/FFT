#include "ruby.h"
#include <math.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif



// Validate the incoming array
static int validate( VALUE self ) {
    long len, i;
    VALUE *values;

    // Check we're an Array
    Check_Type( self, T_ARRAY );

    // make sure the size of the array is a power of 2
    len = RARRAY_LEN( self );
    if( ( len < 2 ) || ( len & ( len - 1 ) ) ) return FALSE;

    // check that we have an appropriate array
    values = RARRAY_PTR( self );
    for( i = 0; i < RARRAY_LEN( self ); i++ ) {
        Check_Type( values[i], T_ARRAY );
        if( RARRAY_LEN( values[i] ) != 2 ) return FALSE;
    }

    return TRUE;
}

static VALUE perform_fft( double **values, long len, int direction ){
    VALUE outArray = rb_ary_new2( len );
    
    long exponent, i, i1, j, k, i2, l, l1, l2;
    double c1,c2,tx,ty,t1,t2,u1,u2,z;

    // Caculate the exponent
    exponent = 0;
    i = len;
    while( i >>= 1 ){
        exponent++;
    }

    // Do the bit reversal
    i2 = len >> 1;
    j = 0;
    for( i = 0; i < ( len - 1 ); i++ ){
        if( i < j ){
            tx = values[i][0];
            ty = values[i][1];
            values[i][0] = values[j][0];
            values[i][1] = values[j][1];
            values[j][0] = tx;
            values[j][1] = ty;
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
            for( i = j; i < len; i += l2){
                i1 = i + l1;
                t1 = u1 * values[i1][0] - u2 * values[i1][1];
                t2 = u1 * values[i1][1] + u2 * values[i1][0];
                values[i1][0] = values[i][0] - t1;
                values[i1][1] = values[i][1] - t2;
                values[i][0] += t1;
                values[i][1] += t2;
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
        for( i = 0; i < len; i++ ){
            values[i][0] /= len;
            values[i][1] /= len;
        }
    }
    // convert the doubles into ruby numbers and stick them into a ruby array
    for( i = 0; i < len; i++ ){
        rb_ary_push( outArray, rb_ary_new3( 2, DBL2NUM( values[i][0] ), DBL2NUM( values[i][1] ) ) );
    }
    
    return outArray;
}

// perform the fft
static VALUE prepare_fft( VALUE inArray, int direction ){
    long i;
    VALUE *values;
    double **transformed;
    VALUE outArray;

    if( !validate( inArray ) ) {
        return Qnil;
    }

    // convert the ruby array into a C array of integers using FIX2INT(Fixnum)
    transformed = malloc( sizeof( double* ) * RARRAY_LEN( inArray ) );
    values = RARRAY_PTR( inArray );
    for( i = 0; i < RARRAY_LEN( inArray ); i++ ) {
        // process values
        transformed[i] = malloc( sizeof( double ) * 2 );
        transformed[i][0] = NUM2DBL( RARRAY_PTR( values[i] )[0] );
        transformed[i][1] = NUM2DBL( RARRAY_PTR( values[i] )[1] );
    }
    
    // do the actual transform
    outArray = perform_fft( transformed, RARRAY_LEN( inArray ), direction );

    for( i = 0; i < RARRAY_LEN( inArray ); i++ ) {
        free( transformed[i] );
    }    
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