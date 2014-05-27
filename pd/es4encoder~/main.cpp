/*
 Copyright (c) 2014 Expert Sleepers Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "m_pd.h"

#include <algorithm>

#if __LP64__
typedef unsigned int                    UInt32;
typedef signed int                      SInt32;
#else
typedef unsigned long                   UInt32;
typedef signed long                     SInt32;
#endif

#define ES_BITSTOFLOAT_SETUP()								\
	float btf_n_factor, btf_p_factor;						\
	btf_n_factor = -(float)0x800000;			btf_p_factor = (float)0x800000;

#define ES_BITSTOFLOAT( bits )	\
	( ( bits & 0x800000 ) ? ( (((float)(0xffffff&(-(SInt32)(bits)))) / btf_n_factor ) ) : ( ((float)(bits)) / btf_p_factor ) )

#define ES_FLOATTOBITS( f )		\
	( ( f < 0 ) ? ( 0x800000 | ( 0xffffff & -(SInt32)( (f) * btf_n_factor ) ) ) : ( (UInt32)( (f) * btf_p_factor ) ) )

// struct to represent the object's state
typedef struct _es4encoder {
	t_object		ob;
	t_sample		m_dummy;
	
	t_inlet*		m_in[4];	// inlets 2-5. inlet 1 is created by CLASS_MAINSIGNALIN
	t_outlet*		m_out[2];
} t_es4encoder;

// method prototypes
extern "C" {
void es4encoder_tilde_setup(void);
void *es4encoder_new(t_symbol *s, int argc, t_atom *argv);
void es4encoder_free(t_es4encoder *x);
void es4encoder_dsp(t_es4encoder *x, t_signal **sp);
t_int *es4encoder_perform(t_int *w);
}

// global class pointer variable
static t_class *es4encoder_class = NULL;

//***********************************************************************************************

void es4encoder_tilde_setup(void)
{
	t_class *c = class_new(gensym("es4encoder~"), (t_newmethod)es4encoder_new, (t_method)es4encoder_free, sizeof(t_es4encoder), CLASS_DEFAULT, A_GIMME, 0);
	
	class_addmethod(c, (t_method)es4encoder_dsp,		gensym("dsp"),		A_CANT, 0);		// Old 32-bit MSP dsp chain compilation for Max 5 and earlier
	
	CLASS_MAINSIGNALIN(c, t_es4encoder, m_dummy);
	
	es4encoder_class = c;
}

void *es4encoder_new(t_symbol *s, int argc, t_atom *argv)
{
	t_es4encoder *x = (t_es4encoder *)pd_new(es4encoder_class);
	
	if (x) {
		for ( int i=0; i<4; ++i )
			x->m_in[i] = inlet_new( &x->ob, &x->ob.ob_pd, &s_signal, &s_signal );
		for ( int i=0; i<2; ++i )
			x->m_out[i] = outlet_new( &x->ob, &s_signal );
	}
	return (x);
}

void es4encoder_free(t_es4encoder *x)
{
	for ( int i=0; i<4; ++i )
		inlet_free( x->m_in[i] );
	for ( int i=0; i<2; ++i )
		outlet_free( x->m_out[i] );
}

//***********************************************************************************************

void es4encoder_dsp(t_es4encoder *x, t_signal **sp)
{
	dsp_add(es4encoder_perform, 9, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
									  sp[5]->s_vec, sp[6]->s_vec,
									  sp[0]->s_n);
}

//***********************************************************************************************

t_int *es4encoder_perform(t_int *w)
{
//	t_es4encoder *x = (t_es4encoder *)(w[1]);
	t_float *in1 = (t_float *)(w[2]);
	t_float *in2 = (t_float *)(w[3]);
	t_float *in3 = (t_float *)(w[4]);
	t_float *in4 = (t_float *)(w[5]);
	t_float *in5 = (t_float *)(w[6]);
	t_float *dstpL = (t_float *)(w[7]);
	t_float *dstpR = (t_float *)(w[8]);
	long n = (long)(w[9]);

	struct {
		int	m_interfaceCategory;
	}	m_preCompNoInterp;
	m_preCompNoInterp.m_interfaceCategory = 0;
	
	ES_BITSTOFLOAT_SETUP()

	while (n--)
	{
		UInt32 out1 = (UInt32)( std::max( 0.0f, std::min( 255.0f, *in1++ ) ) );
		UInt32 out2 = (UInt32)( std::max( 0.0f, std::min( 255.0f, *in2++ ) ) );
		UInt32 out3 = (UInt32)( std::max( 0.0f, std::min( 255.0f, *in3++ ) ) );
		UInt32 out4 = (UInt32)( std::max( 0.0f, std::min( 255.0f, *in4++ ) ) );
		UInt32 out5 = (UInt32)( std::max( 0.0f, std::min( 255.0f, *in5++ ) ) );

		SInt32 bitsL = ( out1 << 16 ) | ( out2 << 8 ) | ( ( out5 << 0 ) & 0xf0 );
		SInt32 bitsR = ( out3 << 16 ) | ( out4 << 8 ) | ( ( out5 << 4 ) & 0xf0 );
		
		float floatL = ES_BITSTOFLOAT( bitsL );
		float floatR = ES_BITSTOFLOAT( bitsR );

		*dstpL++ = floatL;
		*dstpR++ = floatR;
	}

	return w + 10;
}

//***********************************************************************************************
