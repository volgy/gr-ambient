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

#include <ctime>
#include <gnuradio/io_signature.h>
#include "packet_squelch_impl.h"

namespace gr {
  namespace ambient {

    packet_squelch::sptr
    packet_squelch::make(int samples_per_bit)
    {
      return gnuradio::get_initial_sptr
        (new packet_squelch_impl(samples_per_bit));
    }

    /*
     * The private constructor
     */
    packet_squelch_impl::packet_squelch_impl(int smps_per_bit)
      : gr::block("packet_squelch",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
              samples_per_bit(smps_per_bit ), state(FRAME), pulse_cnt(0), prev_sample(0)
    {
      spb_long_pulse = round(20 * samples_per_bit);
    }

    /*
     * Our virtual destructor.
     */
    packet_squelch_impl::~packet_squelch_impl()
    {
    }

    void
    packet_squelch_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = noutput_items;
    }

    int
    packet_squelch_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items_v,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        float *out = (float *) output_items[0];
        int ninput_items = ninput_items_v[0];

        noutput_items = 0; 
        for (int i = 0; i < ninput_items; i++) {
          bool edge = fabs(in[i] - prev_sample) > 0.5; // TODO: presuming threshold block 
          prev_sample = in[i];

          if (edge) {
            switch (state) {
              case IDLE:
                std::cout << now("%D %T") << ">";
                state = FRAME;
                break;
              case FRAME:
                break;
            }
            pulse_cnt = 0;
          }

          if (state != IDLE) {
            pulse_cnt++;
            out[noutput_items++] = in[i];
            if (pulse_cnt > spb_long_pulse) { 
              state = IDLE;
              std::cout << std::endl;
            }
          }
        }

        consume_each (ninput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

    std::string packet_squelch_impl::now( const char* format)
    {
        std::time_t t = std::time(0);
        char cstr[128];
        std::strftime( cstr, sizeof(cstr), format, std::localtime(&t) );
        return cstr;
    }

  } /* namespace ambient */
} /* namespace gr */

