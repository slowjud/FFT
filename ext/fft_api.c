#include "ruby.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif



// Validate the incoming array
static int validate( VALUE self ) {
    int    i;
    int    len;
    VALUE *values;
    
    // Check we're an Array
    Check_Type( self, T_ARRAY );

    // make sure the size of the array is a power of 2
    len = RARRAY_LEN( self );
    if( ( len < 2 ) || ( len & ( len − 1 ) ) ) return FALSE;
        
    // go through the array and check for FIXNUM_P(value)
    values = RARRAY_PTR( self );
    for( i = 0; i < RARRAY_LEN( self ); i++ ) {
        if( !FIXNUM_P( values[i] ) ) return FALSE;
    }

    return TRUE;
}

// perform the fft
static VALUE method_fft( VALUE inArray ){
    int i;
    VALUE *values;
    int *transformed = malloc( sizeof( int ) * RARRAY_LEN( inArray ) );
    VALUE outArray = rb_ary_new2( RARRAY_LEN( inArray ) );

    if( !validate( inArray ) ) {
        return Qnil;
    }

    // convert the ruby array into a C array of integers using FIX2INT(Fixnum)
    values = RARRAY_PTR( inArray );
    for( i = 0; i < RARRAY_LEN( inArray ); i++ ) {
        // process values
        transformed[i] = FIX2INT( values[i] );
    }
    
    // convert the integers into fixnums and stick them into a ruby array
    for( i = 0; i < RARRAY_LEN( inArray ); i++ ){
        rb_ary_push( outArray, INT2FIX( transformed[i] ));
    }
    
    free( transformed );
    
    return outArray;
}

// initalizer
VALUE FFT;
void Init_fft_api() {
    FFT = rb_define_module("FFTAPI");
    rb_define_method(FFT, "fft", method_fft, 0);
}