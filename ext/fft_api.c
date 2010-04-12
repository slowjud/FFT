#include "ruby.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif



// Validate the incoming array
static int validate( VALUE self ) {
    int len;
    int i;
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

static VALUE perform_fft( double *values ){
    
}

// perform the fft
static VALUE prepare_fft( VALUE inArray ){
    int i;
    VALUE *values;
    double **transformed = malloc( sizeof( double* ) * RARRAY_LEN( inArray ) );
    VALUE outArray = rb_ary_new2( RARRAY_LEN( inArray ) );

    if( !validate( inArray ) ) {
        return Qnil;
    }

    // convert the ruby array into a C array of integers using FIX2INT(Fixnum)
    values = RARRAY_PTR( inArray );
    for( i = 0; i < RARRAY_LEN( inArray ); i++ ) {
        // process values
        transformed[i] = malloc( sizeof( double ) * 2 );
        transformed[i][0] = NUM2DBL( RARRAY_PTR( values[i] )[0] );
        transformed[i][1] = NUM2DBL( RARRAY_PTR( values[i] )[1] );
    }
    
    // convert the doubles into ruby numbers and stick them into a ruby array
    for( i = 0; i < RARRAY_LEN( inArray ); i++ ){
        rb_ary_push( outArray, rb_ary_new3( 2, DBL2NUM( transformed[i][0] ), DBL2NUM( transformed[i][1] ) ) );
    }
    
    free( transformed );
    
    return outArray;
}

// initalizer
VALUE FFT;
void Init_fft_api() {
    FFT = rb_define_module("FFTAPI");
    rb_define_method(FFT, "fft", prepare_fft, 0);
}