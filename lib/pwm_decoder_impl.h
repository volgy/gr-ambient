/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_AMBIENT_PWM_DECODER_IMPL_H
#define INCLUDED_AMBIENT_PWM_DECODER_IMPL_H

#include <ambient/pwm_decoder.h>

namespace gr {
  namespace ambient {

    class pwm_decoder_impl : public pwm_decoder
    {
     private:
      int state;
      float threshold;
      int zero_min, zero_max;
      int one_min, one_max;
      unsigned int width;

     public:
      pwm_decoder_impl(int zero_len, int one_len, float slack, float threshold);
      ~pwm_decoder_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace ambient
} // namespace gr

#endif /* INCLUDED_AMBIENT_PWM_DECODER_IMPL_H */

