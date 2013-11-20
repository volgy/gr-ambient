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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "pwm_decoder_impl.h"

namespace gr {
  namespace ambient {

    pwm_decoder::sptr
    pwm_decoder::make(int zero_len, int one_len, float slack, float threshold)
    {
      return gnuradio::get_initial_sptr
        (new pwm_decoder_impl(zero_len, one_len, slack, threshold));
    }

    /*
     * The private constructor
     */
    pwm_decoder_impl::pwm_decoder_impl(int zero_len, int one_len, float slack, float threshold)
      : gr::sync_block("pwm_decoder",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)), 
        state(0), width(0)
    {
      zero_min = zero_len * (1.0 - slack);
      zero_max = ceil(zero_len * (1.0 + slack));
      one_min = one_len * (1.0 - slack);
      one_max = ceil(one_len * (1.0 + slack));
      std::cout << "pwm_decoder zero: " << zero_min << " - " << zero_max << 
                               ", one: " << one_min << " - " << one_max << std::endl;
    }

    /*
     * Our virtual destructor.
     */
    pwm_decoder_impl::~pwm_decoder_impl()
    {
    }

    int
    pwm_decoder_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        for (int i = 0; i < noutput_items; i++) {
          if (state == 0) {
            if (in[i] > threshold) {
              width = 1;
              state = 1;
            }
            else {
              width++;
            }
          }
          else {
            if (in[i] < threshold) {
              //std::cout << "[" << width << "]";
              if (width >= zero_min && width <= zero_max) {
                std::cout << "0 ";
              }
              if (width >= one_min && width <= one_max) {
                std::cout << "1 ";
              }
              width = 1;
              state = 0;
            }
            else {
              width++;
            }
          }
          if (width == 1e4) {
            std::cout << std::endl;
          }
        }

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace ambient */
} /* namespace gr */

