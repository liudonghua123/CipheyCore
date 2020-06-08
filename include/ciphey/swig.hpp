#pragma once

#include <ciphey/ciphers.hpp>

#include <memory>

namespace ciphey {
  // We use the same names for the various analysis functions to simplify the API

  // +-------------------------------------------------------------------------+
  // |                            SIMPLE ANALYSIS                              |
  // +-------------------------------------------------------------------------+
  struct simple_analysis_res {
    freq_table freqs;
    freq_t len = 0;
  };
  inline std::shared_ptr<simple_analysis_res> analyse_string(string_t str) {
    auto ret = std::make_shared<simple_analysis_res>();
    ret->len = str.size();
    freq_analysis(ret->freqs, str);
//    ret->probs = freq_conv(ret->freqs, ret->len);
    return ret;
  }
  inline std::shared_ptr<simple_analysis_res> start_analysis() {
    auto ret = std::make_shared<simple_analysis_res>();
    return ret;
  }
  inline void continue_analysis(std::shared_ptr<simple_analysis_res> target, string_t str) {
    freq_analysis(target->freqs, str);
    target->len += str.size();
  }
//  inline void finish_analysis(std::shared_ptr<simple_analysis_res> target) {
//    target->probs = freq_conv(target->freqs, target->len);
//  }

  // +-------------------------------------------------------------------------+
  // |                           WINDOWED ANALYSIS                             |
  // +-------------------------------------------------------------------------+
  struct windowed_analysis_res {
    windowed_freq_table freqs;
    freq_t len = 0;
  };
  inline std::shared_ptr<windowed_analysis_res> analyse_string(string_t str, size_t window_size) {
    auto ret = std::make_shared<windowed_analysis_res>();
    ret->len = str.size();
    ret->freqs.resize(window_size);
    freq_analysis(ret->freqs, str);
//    ret->probs = freq_conv(ret->freqs, ret->len);
    return ret;
  }
  inline std::shared_ptr<simple_analysis_res> start_analysis(size_t window_size) {
    auto ret = std::make_shared<simple_analysis_res>();
    return ret;
  }
  inline void continue_analysis(std::shared_ptr<windowed_analysis_res> target, string_t str) {
    freq_analysis(target->freqs, str, target->len);
    target->len += str.size();
  }
//  inline void finish_analysis(std::shared_ptr<windowed_analysis_res> target) {
//    target->probs = freq_conv(target->freqs, target->len);
//  }

  // +-------------------------------------------------------------------------+
  // |                               STATISTICS                                |
  // +-------------------------------------------------------------------------+

  // TODO: add noise param
  inline string_t fuzz(std::shared_ptr<simple_analysis_res> const& in, size_t len) {
    return generate_fuzz(freq_conv(in->freqs, in->len), len);
  }

  inline prob_t chisq_test(std::shared_ptr<simple_analysis_res> in, prob_table expected) {
    return gof_chisq(create_assoc_table(freq_conv(in->freqs, in->len), expected), in->len);
  }

  // +-------------------------------------------------------------------------+
  // |                              CRYPTOGRAPHY                               |
  // +-------------------------------------------------------------------------+
  inline std::vector<ciphey::crack_result<ciphey::caesar::key_t>> caesar_crack(std::shared_ptr<simple_analysis_res> in,
                                                                               prob_table expected, group_t group,
                                                                               bool do_filter_missing = true,
                                                                               prob_t p_value = default_p_value) {
    if (do_filter_missing) {
      auto tab = in->freqs;
      size_t new_len = in->len - filter_missing(tab, expected);
      return caesar::crack(freq_conv(tab, new_len), expected, group, new_len, p_value);
    }

    return caesar::crack(freq_conv(in->freqs, in->len), expected, group, in->len, p_value);
  }

  inline string_t caesar_decrypt(string_t str, ciphey::caesar::key_t key, group_t group) {
    caesar::decrypt(str, key, group);
    return str;
  }
  inline string_t caesar_encrypt(string_t str, ciphey::caesar::key_t key, group_t group) {
    caesar::encrypt(str, key, group);
    return str;
  }
  inline prob_t caesar_detect(std::shared_ptr<simple_analysis_res> in, prob_table expected) {
    return caesar::detect(in->probs, expected, in->len);
  }

  inline std::vector<ciphey::crack_result<ciphey::vigenere::key_t>> vigenere_crack(std::shared_ptr<windowed_analysis_res> in,
                                                                                   prob_table const& expected,
                                                                                   group_t const& group,
                                                                                   bool do_filter_missing = true,
                                                                                   prob_t p_value = default_p_value) {
    if (do_filter_missing) {
      auto tab = in->freqs;
      size_t new_len = in->len - filter_missing(tab, expected);
      return vigenere::crack(freq_conv(tab, new_len), expected, group, new_len, p_value);
    }

    return vigenere::crack(freq_conv(in->freqs, in->len), expected, group, in->len, p_value);
  }

  inline string_t vigenere_decrypt(string_t str, ciphey::vigenere::key_t key, group_t group) {
    vigenere::decrypt(str, key, group);
    return str;
  }

  inline string_t vigenere_encrypt(string_t str, ciphey::vigenere::key_t key, group_t group) {
    vigenere::encrypt(str, key, group);
    return str;
  }

//  inline data xor_single_crypt(data str, ciphey::xor_single::key_t key) {
//    xor_single::crypt(str, key);
//    return str;
//  }
}
