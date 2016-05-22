/*
* rtl_power_fftw, program for calculating power spectrum from rtl-sdr reciever.
* Copyright (C) 2015 Klemen Blokar <klemen.blokar@ad-vega.si>
*                    Andrej Lajovic <andrej.lajovic@ad-vega.si>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RTL_H
#define RTL_H

#include <vector>
#include <rtl-sdr.h>

#include "datastore.h"

//enum gain_types { total, lna, mixer, vga };

class Rtlsdr {
public:
  Rtlsdr(int dev_index);
  ~Rtlsdr();
  // Rtlsdr objects cannot be copied; they can be moved, though.
  Rtlsdr(const Rtlsdr&) = delete;
  Rtlsdr& operator=(const Rtlsdr&) = delete;

  // Reading parameters & data.
  std::vector<int> gains() const;
  std::vector<int> lna_gains() const;
  std::vector<int> mixer_gains() const;
  std::vector<int> vga_gains() const;
  std::vector<int> specific_gains(gain_types gain_type) const;
  uint32_t sample_rate() const;
  uint32_t frequency() const ;
  bool read(Buffer& buffer) const;

  // Parameter setting.
  void set_gain(int gain);
  void set_lna_gain(int gain);
  void set_mixer_gain(int gain);
  void set_vga_gain(int gain);
  void set_frequency(uint32_t frequency);
  void set_freq_correction(int ppm_error);
  void set_sample_rate(uint32_t sample_rate);

  // Convenience functions.
  int nearest_gain(int gain);
  int nearest_lna_gain(int gain);
  int nearest_mixer_gain(int gain);
  int nearest_vga_gain(int gain);
  int nearest_specific_gain(int gain, gain_types gain_type);

  void print_gains() const;
  void print_lna_gains() const;
  void print_mixer_gains() const;
  void print_vga_gains() const;
  void print_specific_gains(gain_types gain_type) const;

private:
  rtlsdr_dev_t *dev;
};

#endif // RTL_H
