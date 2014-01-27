/* -*- c++ -*- */

#define AMBIENT_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "ambient_swig_doc.i"

%{
#include "ambient/manchester_decoder.h"
#include "ambient/packet_squelch.h"
%}

%include "ambient/manchester_decoder.h"
GR_SWIG_BLOCK_MAGIC2(ambient, manchester_decoder);
%include "ambient/packet_squelch.h"
GR_SWIG_BLOCK_MAGIC2(ambient, packet_squelch);
