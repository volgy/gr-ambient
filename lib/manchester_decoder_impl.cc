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
#include <boost/format.hpp>
#include <gnuradio/io_signature.h>
#include "manchester_decoder_impl.h"

 #define min(a,b) ( (a)<(b) ? (a) : (b))

//#define PRINT_BITS
#define PRINT_MEASUREMENT

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
      samples_per_bit(smps_per_bit ), state(IDLE), pulse_width(0), 
      prev_sample(0), frame_len(0), frame_ts("")
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
                frame_ts = now("%D %T");
                frame_len = 0;
                pulse_width = round(0.5 * samples_per_bit); // this is a clock edge, preparing for '1'
                state = FRAME;
                break;
              case FRAME:
                if (pulse_width > spb_short_pulse) {
                  frame[frame_len] = edge_hl;
                  frame_len = min(frame_len + 1, sizeof(frame)); // inc w/ overflow protection
                  pulse_width = 0;
                }
                break;
            }
          }

          if (state != IDLE) {
            pulse_width++;
            if (pulse_width > spb_long_pulse) { 
              state = IDLE;
              if (frame_len > 0) {  // Filter turn-on transients
                //dump_bits();
                dump_measurement();
              }
            }
          }
          
        }
        return noutput_items;
    }

    std::string manchester_decoder_impl::now( const char* format)
    {
        std::time_t t = std::time(0);
        char cstr[128];
        std::strftime( cstr, sizeof(cstr), format, std::localtime(&t) );
        return cstr;
    }

    void manchester_decoder_impl::dump_bits()
    {
      std::cout << frame_ts << "> ";
      for (int i = 0; i < frame_len; i++) {
        std::cout << (frame[i] ? "1" : "0");
      }
      std::cout << "(len=" << frame_len << ")" << std::endl;
      std::cout << std::flush;
    }

    int manchester_decoder_impl::slice(int from, int len)
    {
      int val = 0;

      if (from < 0 || (from + len) >= sizeof(frame)) {
        return -1;
      }
      while (len--) {
        val = (val << 1) + frame[from++];
      } 

      return val;
    }

    // TODO: eventually this should be a separate block
    //       does not belong to this level (Manchester decoding)
    void manchester_decoder_impl::dump_measurement()
    {
      const int AMBIENT_FRAME_LEN = 195;
      const int AMBIENT_PACKET_LEN = 65;

      std::cout << frame_ts << "> ";

      if (frame_len != AMBIENT_FRAME_LEN) {
        std::cout << "ERROR: invalid frame length (" << frame_len << ")" << std::endl;
        return;
      }

      // Each packet is repeated 3 times
      for (int i = 0; i < AMBIENT_PACKET_LEN; i++) {
        if ((frame[i] != frame[i+AMBIENT_PACKET_LEN]) ||
            (frame[i] != frame[i+2*AMBIENT_PACKET_LEN])) {
          
          std::cout << "ERROR: invalid frame (mismatch)" << std::endl;
        }
      }

      int r = slice(22, 8);
      int c = slice(30, 3); 
      int t = slice(34, 12);
      int h = slice(46, 8);

      int channel = c + 1;
      float temp = t/20.0-40;
      int humidity = h/2;

      std::cout << boost::format("CH%d [%d] Temp=%3.1fF RH=%2d%%\n") \\
                                  % channel % r % temp % humidity;
      std::cout << std::flush;
    }

  } /* namespace ambient */
} /* namespace gr */

