// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.03.28

#pragma once

#include <petuum_ps_common/include/petuum_ps.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "updateScheduler.hpp"
#include "common.hpp"

#include "document_word_topics.hpp"

namespace lda {

typedef double real_t;

// FastDocSampler samples one document at a time using Yao's fast sampler
// (http://people.cs.umass.edu/~lmyao/papers/fast-topic-model10.pdf section
// 3.4. We try to keep the variable names consistent with the paper.)
// FastDocSampler does not store any documents.
class FastDocSamplerVirtual {
public:
  FastDocSamplerVirtual();
  ~FastDocSamplerVirtual();

  void SampleOneDoc(DocumentWordTopics* doc);

private:  // private members.
  // ================ Topic Model Parameters =================
  // Number of topics.
  int32_t K_;

  // Number of vocabs.
  int32_t V_;

  // Dirichlet prior for word-topic vectors.
  real_t beta_;

  // Equals to V_*beta_
  real_t beta_sum_;

  // Dirichlet prior for doc-topic vectors.
  real_t alpha_;

  // Word topic table of V_ rows, each of which is a [K x 1] dim sparse sorted
  // row.
  petuum::Table<int32_t> word_topic_table_;

  // ================== Utilities ====================
  typedef boost::variate_generator<boost::mt19937&,
          boost::uniform_real<real_t> > rng_t;

  boost::mt19937 rng_engine_;

  boost::uniform_real<real_t> uniform_zero_one_dist_;

  // zero_one_rng_ generates random real on [0, 1)
  std::unique_ptr<rng_t> zero_one_rng_;


  // =================== georg's additions ===========
  std::vector<int32_t> summary_vals_;
  std::vector<std::map<int32_t,int16_t> > word_topic_vals_;
  std::vector<std::map<int32_t,int16_t> > word_topic_delta_;
  std::vector<real_t> s_vector_;
  real_t s_total_;
public:
  int thread_id;

public:
  void pull(RowUpdateItem item);
  void push(RowUpdateItem item);
  void buildGlobalCache();
  void updateGlobalCache();  
  std::map<int32_t,int16_t> get_word_topic_row(int wordId);
  std::vector<int32_t> get_summary_vals();

private:
  void pullRow(int32_t index, std::map<int32_t,int16_t> * changes);
  void pushRow(int32_t index);
  void pullMany(std::vector<int32_t> indeces);
  void pushMany(std::vector<int32_t> indeces);

};

}  // namespace lda









































