/* -*- c++ -*- */

#define AMBIENT_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "ambient_swig_doc.i"

%{
#include "ambient/pwm_decoder.h"
%}


%include "ambient/pwm_decoder.h"
GR_SWIG_BLOCK_MAGIC2(ambient, pwm_decoder);