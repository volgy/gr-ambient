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
#include "manchester_decoder_impl.h"

//#define DIFFERENTIAL


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
          spb_short_pulse = round(0.7 * samples_per_bit);
          spb_long_pulse = round(1.2 * samples_per_bit);
    }

    /*
     * Our virtual destructor.
     */
    manchester_decoder_impl::~manchester_decoder_impl()
    {
    }

#ifdef DIFFERENTIAL

    int
    manchester_decoder_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        for (int i = 0; i < noutput_items; i++) {
          bool change = fabs(in[i] - prev_sample) > 0.5; // TODO: presuming threshold block
          prev_sample = in[i];

          if (change) {
            switch (state) {
              case IDLE:
                //std::cout << "manchester_decoder: SOF " << now() << std::endl;
                std::cout << now("%D %T") << "> ";
                bit_cnt = 0;
                emit_bit(0);
                state = CLOCK;
                break;
              case CLOCK:
                if (pulse_cnt > spb_short_pulse) {
                  std::cout << "manchester_decoder: clock recovery error, dropping frame" << std::endl;
                  state = IDLE;
                }
                else {
                  state = DATA;
                }
                break;
              case DATA:
                if (pulse_cnt > spb_short_pulse) {
                  emit_bit(1);
                  state = DATA;
                }
                else {
                  emit_bit(0);
                  state = CLOCK;
                }
                break;
            }
            pulse_cnt = 0;
          }

          if (state != IDLE) {
            pulse_cnt++;
            if (pulse_cnt > spb_long_pulse) { 
              state = IDLE;
              /*
              std::cout << std::endl << "manchester_decoder: timeout (EOF) " \
                << (state == DATA ? "data" : "clock") \
                << " " << bit_cnt << std::endl;
              */
              std::cout << " (len=" << bit_cnt << ")" << std::endl;
            }
          }
          
        }
        return noutput_items;
    }

#else // NON-DIFFERENTIAL

    int
    manchester_decoder_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        for (int i = 0; i < noutput_items; i++) {
          bool edge_lh = (in[i] - prev_sample) > 0.5; // TODO: presuming threshold block
          bool edge_hl = (prev_sample - in[i]) > 0.5; // TODO: presuming threshold block
          bool edge = edge_lh || edge_hl; 
          prev_sample = in[i];

          if (edge) {
            switch (state) {
              case IDLE:
                //std::cout << "manchester_decoder: SOF " << now() << std::endl;
                std::cout << now("%D %T") << "> ";
                bit_cnt = 0;
                pulse_cnt = round(0.5 * samples_per_bit);
                //pulse_cnt = 0; emit_bit(edge_lh ? 1 : 0);  // TODO: configurable polarity
                state = DATA;
                break;
              case DATA:
                if (pulse_cnt > spb_short_pulse) {
                  emit_bit(edge_lh ? 1 : 0);  // TODO: configurable polarity
                  pulse_cnt = 0;
                }
                break;
            }
          }

          if (state != IDLE) {
            pulse_cnt++;
            if (pulse_cnt > spb_long_pulse) { 
              state = IDLE;
              /*
              std::cout << std::endl << "manchester_decoder: timeout (EOF) " \
                << (state == DATA ? "data" : "clock") \
                << " " << bit_cnt << std::endl;
              */
              std::cout << " (len=" << bit_cnt << ")" << std::endl;
            }
          }
          
        }
        return noutput_items;
    }

#endif  // DIFFERENTIAL vs. NON-DIFFERENTIAL

    std::string manchester_decoder_impl::now( const char* format)
    {
        std::time_t t = std::time(0);
        char cstr[128];
        std::strftime( cstr, sizeof(cstr), format, std::localtime(&t) );
        return cstr;
    }

    void manchester_decoder_impl::emit_bit(int bit) 
    {
      std::cout << (bit ? "1" : "0");
      if ( ((++bit_cnt) % 4) == 0 ) {
        std::cout << " ";
      }
    } 

  } /* namespace ambient */
} /* namespace gr */

