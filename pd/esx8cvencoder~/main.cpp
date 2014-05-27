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

// struct to represent the object's state
typedef struct _esx8cvencoder {
	t_object		ob;
	int				m_phase;
	UInt32			m_value;
	t_sample		m_dummy;

	t_inlet*		m_in[7];	// inlets 2-8. inlet 1 is created by CLASS_MAINSIGNALIN
	t_outlet*		m_out[1];
} t_esx8cvencoder;

// method prototypes
extern "C" {
void esx8cvencoder_tilde_setup(void);
void *esx8cvencoder_new(t_symbol *s, long argc, t_atom *argv);
void esx8cvencoder_free(t_esx8cvencoder *x);
void esx8cvencoder_dsp(t_esx8cvencoder *x, t_signal **sp);
t_int *esx8cvencoder_perform(t_int *w);
}

// global class pointer variable
static t_class *esx8cvencoder_class = NULL;

//***********************************************************************************************

void esx8cvencoder_tilde_setup(void)
{
	t_class *c = class_new(gensym("esx8cvencoder~"), (t_newmethod)esx8cvencoder_new, (t_method)esx8cvencoder_free, (long)sizeof(t_esx8cvencoder), CLASS_DEFAULT, A_GIMME, 0);
	
	class_addmethod(c, (t_method)esx8cvencoder_dsp,		gensym("dsp"),		A_CANT, 0);		// Old 32-bit MSP dsp chain compilation for Max 5 and earlier
	
	CLASS_MAINSIGNALIN(c, t_esx8cvencoder, m_dummy);

	esx8cvencoder_class = c;
}

void *esx8cvencoder_new(t_symbol *s, long argc, t_atom *argv)
{
	t_esx8cvencoder *x = (t_esx8cvencoder *)pd_new(esx8cvencoder_class);
	
	if (x) {
		x->m_phase = 0;
		x->m_value = 0;
		for ( int i=0; i<7; ++i )
			x->m_in[i] = inlet_new( &x->ob, &x->ob.ob_pd, &s_signal, &s_signal );
		for ( int i=0; i<1; ++i )
			x->m_out[i] = outlet_new( &x->ob, &s_signal );
	}
	return (x);
}

void esx8cvencoder_free(t_esx8cvencoder *x)
{
	for ( int i=0; i<7; ++i )
		inlet_free( x->m_in[i] );
	for ( int i=0; i<1; ++i )
		outlet_free( x->m_out[i] );
}

//***********************************************************************************************

void esx8cvencoder_dsp(t_esx8cvencoder *x, t_signal **sp)
{
	dsp_add(esx8cvencoder_perform, 11, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec,
			sp[8]->s_vec,
			sp[0]->s_n);
}

//***********************************************************************************************

t_int *esx8cvencoder_perform(t_int *w)
{
	t_esx8cvencoder *x = (t_esx8cvencoder *)(w[1]);
	t_float* srcp[8];
	srcp[0] = (t_float *)(w[2]);
	srcp[1] = (t_float *)(w[3]);
	srcp[2] = (t_float *)(w[4]);
	srcp[3] = (t_float *)(w[5]);
	srcp[4] = (t_float *)(w[6]);
	srcp[5] = (t_float *)(w[7]);
	srcp[6] = (t_float *)(w[8]);
	srcp[7] = (t_float *)(w[9]);
	t_float *dstp = (t_float *)(w[10]);
	long n = (long)(w[11]);

	int phase = x->m_phase;
	UInt32 value = x->m_value;
	bool smuxProof = false;
	if ( !smuxProof )
		phase = phase & ~1;
	int phaseInc = smuxProof ? 1 : 2;

	double values[8];

	while (n--)
	{
		for ( int i=0; i<8; ++i )
		{
			values[i] = *srcp[i]++;
		}
		
		int state = ( phase >> 1 ) & 3;
		int dac = ( phase >> 3 ) & 7;
		
		if ( state == 0 )
		{
			{
				// take the next dac in sequence
				double s = values[ dac ];
				value = 2048 + (SInt32)( std::max( -2048.0, std::min( 2047.0, s ) ) );
			}
		}
		
		phase += phaseInc;
		if ( ( phase & 7 ) == 6 )
			phase += 2;
		phase = phase & 63;
		
		UInt32 out = ( state == 0 ) ? ( 0x80 | ( value & 0x1f ) )
		: ( ( state == 1 ) ? ( ( value >> 5 ) & 0x1f )
		   : ( ( ( dac > 3 ) ? 0x40 : 0x20 ) | ( value >> 10 ) | ( ( dac & 3 ) << 2 ) ) );
		
		*dstp++ = (float)out;
	}

	x->m_phase = phase;
	x->m_value = value;

	return w + 12;
}

//***********************************************************************************************
