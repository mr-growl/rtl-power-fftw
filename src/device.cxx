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

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

#include "device.h"
#include "exceptions.h"


Rtlsdr::Rtlsdr(int dev_index) {
  int num_of_rtls = rtlsdr_get_device_count();
  if (num_of_rtls == 0) {
    throw RPFexception(
      "No RTL-SDR compatible devices found.",
      ReturnValue::NoDeviceFound);
  }

  if (dev_index >= num_of_rtls) {
    throw RPFexception(
      "Invalid RTL device number. Only " + std::to_string(num_of_rtls) + " devices available.",
      ReturnValue::InvalidDeviceIndex);
  }

  int rtl_retval = rtlsdr_open(&dev, (uint32_t)dev_index);
  if (rtl_retval < 0 ) {
    throw RPFexception(
      "Could not open rtl_sdr device " + std::to_string(dev_index),
      ReturnValue::HardwareError);
  }
}

Rtlsdr::~Rtlsdr() {
  rtlsdr_close(dev);
}

std::vector<int> Rtlsdr::gains() const {
	return specific_gains(total);
}
std::vector<int> Rtlsdr::lna_gains() const {
	return specific_gains(lna);
}
std::vector<int> Rtlsdr::mixer_gains() const {
	return specific_gains(mixer);
}
std::vector<int> Rtlsdr::vga_gains() const {
	return specific_gains(vga);
}

std::vector<int> Rtlsdr::specific_gains(enum gain_types gain_type) const {
  //Set up a pointer to the "get gains" function and set it in the switch
  int (*ptr_rtlsdr_get_gains)(rtlsdr_dev_t*,int*);
  switch(gain_type)
  {
    case lna:
      ptr_rtlsdr_get_gains = &rtlsdr_get_lna_gains;
      break;
    case mixer:
      ptr_rtlsdr_get_gains = &rtlsdr_get_mixer_gains;
      break;
    case vga:
      ptr_rtlsdr_get_gains = &rtlsdr_get_vga_gains;
      break;
    default:
      ptr_rtlsdr_get_gains = &rtlsdr_get_tuner_gains;
  }
  int number_of_gains = ptr_rtlsdr_get_gains(dev, NULL);
  if (number_of_gains <= 0) {
    throw RPFexception(
      "RTL device: could not read the number of available gains.",
      ReturnValue::HardwareError);
  }
  std::vector<int> gains(number_of_gains);
  if (ptr_rtlsdr_get_gains(dev, gains.data()) <= 0) {
    throw RPFexception(
      "RTL device: could not retrieve gain values.",
      ReturnValue::HardwareError);
  }
  return gains;
}

uint32_t Rtlsdr::sample_rate() const {
  uint32_t sample_rate = rtlsdr_get_sample_rate(dev);
  if (sample_rate == 0) {
    throw RPFexception(
      "RTL device: could not read sample rate.",
      ReturnValue::HardwareError);
  }
  return sample_rate;
}

uint32_t Rtlsdr::frequency() const {
  uint32_t frequency = rtlsdr_get_center_freq(dev);
  if (frequency == 0) {
    throw RPFexception(
      "RTL device: could not read frequency.",
      ReturnValue::HardwareError);
  }
  return frequency;
}

bool Rtlsdr::read(Buffer& buffer) const {
  int n_read;
  rtlsdr_reset_buffer(dev);
  rtlsdr_read_sync(dev, buffer.data(), buffer.size(), &n_read);
  return n_read == (signed)buffer.size();
}

void Rtlsdr::set_gain(int gain) {
  int status = 0;
  status += rtlsdr_set_tuner_gain_mode(dev, 1);
  status += rtlsdr_set_tuner_gain(dev, gain);

  if (status != 0) {
    throw RPFexception(
      "RTL device: could not set gain.",
      ReturnValue::HardwareError);
  }
}

void Rtlsdr::set_lna_gain(int gain) {
  int status = 0;
  status += rtlsdr_set_lna_gain(dev, gain);

  if (status != 0) {
    throw RPFexception(
      "RTL device: could not set lna gain.",
      ReturnValue::HardwareError);
  }
}

void Rtlsdr::set_mixer_gain(int gain) {
  int status = 0;
  status += rtlsdr_set_mixer_gain(dev, gain);

  if (status != 0) {
    throw RPFexception(
      "RTL device: could not set mixer gain.",
      ReturnValue::HardwareError);
  }
}

void Rtlsdr::set_vga_gain(int gain) {
  int status = 0;
  status += rtlsdr_set_vga_gain(dev, gain);

  if (status != 0) {
    throw RPFexception(
      "RTL device: could not set vga gain.",
      ReturnValue::HardwareError);
  }
}

void Rtlsdr::set_frequency(uint32_t frequency) {
  if (rtlsdr_set_center_freq(dev, frequency) < 0) {
    throw RPFexception(
      "RTL device: could not set center frequency.",
      ReturnValue::HardwareError);
  }
  // This sleeping is inherited from other code. There have been hints of strange
  // behaviour if it was commented out, so we left it in. If you actually know
  // why this would be necessary (or, to the contrary, that it is complete
  // bullshit), you are most welcome to explain it here!
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void Rtlsdr::set_freq_correction(int ppm_error) {
  if (rtlsdr_set_freq_correction(dev, ppm_error) < 0) {
    throw RPFexception(
      "RTL device: could not set frequency correction.",
      ReturnValue::HardwareError);
  }
}

void Rtlsdr::set_sample_rate(uint32_t sample_rate) {
  if (rtlsdr_set_sample_rate(dev, sample_rate)) {
    throw RPFexception(
      "RTL device: could not set sample rate.",
      ReturnValue::HardwareError);
  }
}

int Rtlsdr::nearest_gain(int gain) {
  return nearest_specific_gain(gain, total);
}

int Rtlsdr::nearest_lna_gain(int gain) {
  return nearest_specific_gain(gain, lna);
}

int Rtlsdr::nearest_mixer_gain(int gain) {
  return nearest_specific_gain(gain, mixer);
}

int Rtlsdr::nearest_vga_gain(int gain) {
  return nearest_specific_gain(gain, vga);
}

int Rtlsdr::nearest_specific_gain(int gain, gain_types gain_type) {
  std::vector<int> gain_list;
  switch(gain_type)
  {
    case lna:
      gain_list = lna_gains();
      break;
    case mixer:
      gain_list = mixer_gains();
      break;
    case vga:
      gain_list = vga_gains();
      break;
    case total:
    default:
      gain_list = gains();
  }
  int dif = std::numeric_limits<int>::max();
  int selected = 0;
  for (const auto& trial_gain : gain_list) {
    int temp = abs(trial_gain - gain);
    if ( temp < dif ) {
      dif = temp;
      selected = trial_gain;
    }
  }
  return selected;
}

void Rtlsdr::print_gains() const {
	print_specific_gains(total);
}
void Rtlsdr::print_lna_gains() const {
	print_specific_gains(lna);
}
void Rtlsdr::print_mixer_gains() const {
	print_specific_gains(mixer);
}
void Rtlsdr::print_vga_gains() const {
	print_specific_gains(vga);
}
void Rtlsdr::print_specific_gains(gain_types gain_type) const {
  std::vector<int> gain_table;

  char titles[][7] = {"lna ","mixer ","vga ",""};
  int title_index = 3;

  switch(gain_type)
  {
    case lna:
      title_index = 0;
      gain_table = specific_gains(gain_type);
      break;
    case mixer:
      title_index = 1;
      gain_table = specific_gains(gain_type);
      break;
    case vga:
      title_index = 2;
      gain_table = specific_gains(gain_type);
      break;
    case total:
      title_index = 3;
      gain_table = specific_gains(gain_type);
      break;
    default: //just in case
     gain_table = gains();
  }

  std::cerr << "Available " << titles[title_index] << "gains (in 1/10th of dB): ";
  for (unsigned int i = 0; i < gain_table.size(); i++) {
    if (i != 0)
      std::cerr << ", ";
    std::cerr << gain_table[i];
  }
  std::cerr << std::endl;
}

