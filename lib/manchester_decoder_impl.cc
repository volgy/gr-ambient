/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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
#include "manchester_decoder_impl.h"

namespace gr {
  namespace ambient {

    manchester_decoder::sptr
    manchester_decoder::make(int samples_per_bit)
    {
      return gnuradio::get_initial_sptr
        (new manchester_decoder_impl(samples_per_bit));
    }

    /*
     * The private constructor
     */
    manchester_decoder_impl::manchester_decoder_impl(int smps_per_bit)
      : gr::sync_block("manchester_decoder",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)),
      samples_per_bit(smps_per_bit ), state(IDLE), pulse_cnt(0), prev_sample(0)
    {
    }

    /*
     * Our virtual destructor.
     */
    manchester_decoder_impl::~manchester_decoder_impl()
    {
    }

    int
    manchester_decoder_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];

        for (int i = 0; i < noutput_items; i++) {
          bool change = abs(in[i] - prev_sample) > 0.5; // TODO: presuming threshold block
          prev_sample = in[i];

          if (change) {
            switch (state) {
              case IDLE:
                if (change) {
                  std::cout << "manchester_decoder: change w=" << pulse_cnt << std::endl;
                  pulse_cnt = 0;
                }
                break;
              case CLOCK:
                break;
              case DATA:
                break;
            }
            pulse_cnt = 0;
          }

          if (pulse_cnt > (2*samples_per_bit)) {  // safety margin
            state = IDLE;
            std::cout << "manchester_decoder: timeout (end of packet)" << std::endl;
          }

          if (state != IDLE) {
            pulse_cnt++;
          }
          
        }
        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace ambient */
} /* namespace gr */

