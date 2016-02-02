// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.03.28

#include "fast_doc_sampler_virtual.hpp"
#include "context.hpp"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <petuum_ps_common/include/system_gflags_declare.hpp>
#include <glog/logging.h>
#include <algorithm>
#include <time.h>
#include <sstream>
#include <string>
#include <algorithm>
#include "common.hpp"

namespace lda {

FastDocSamplerVirtual::FastDocSamplerVirtual() :
    rng_engine_(time(NULL)),
    uniform_zero_one_dist_(0, 1),
    zero_one_rng_(new rng_t(rng_engine_, uniform_zero_one_dist_)) {
  Context& context = Context::get_instance();
  // Topic model parameters.
  K_ = context.get_int32("num_topics");
  V_ = context.get_int32("num_vocabs");
  CHECK_NE(-1, V_);
  beta_ = context.get_double("beta");
  beta_sum_ = beta_ * V_;
  alpha_ = context.get_double("alpha");

  // PS tables.
  int32_t word_topic_table_id = context.get_int32("word_topic_table_id");
  word_topic_table_ = petuum::PSTableGroup::GetTableOrDie<int>(word_topic_table_id);
}

FastDocSamplerVirtual::~FastDocSamplerVirtual() {}

void FastDocSamplerVirtual::SampleOneDoc(DocumentWordTopics* doc) 
{
  //checking variables
  double qCounter = 0;
  double sCounter = 0;
  double rCounter = 0;
  // Build document-wise cache
  std::vector<int16_t> docTopicVector(K_, 0);
  std::set<int32_t> docTopicSet;
  for (WordOccurrenceIterator it(doc); !it.End(); it.Next()) 
  {
    ++docTopicVector[it.Topic()];
    docTopicSet.insert(it.Topic());
  }
  std::vector<int32_t> docTopics(docTopicSet.begin(), docTopicSet.end());
  int numDocTopics = (int)docTopics.size();
  real_t r_total = 0.;
  std::vector<real_t> r_vector(K_,0.);
  std::vector<real_t> q_coeffs(K_,0.);
  for(int k=0;k<K_;k++)
  {
    real_t denom = ((real_t)summary_vals_[k]) + beta_sum_;
    q_coeffs[k] = (alpha_ + ((real_t)docTopicVector[k])) / denom;
    if(docTopicSet.count(k) == 1)
    {
      r_vector[k] = (beta_ * ((real_t)docTopicVector[k])) / denom;
      r_total += r_vector[k];
    }
  }

  // Start sampling.
  for (WordOccurrenceIterator it(doc); !it.End(); it.Next()) 
  {
    int32_t oldTopic = it.Topic();
    int32_t wordId = it.Word();
    CHECK_LT(oldTopic, K_);
    //Remove this variable assignment by updating the cache
    //global cache
    summary_vals_[oldTopic]--;
    CHECK(word_topic_vals_[wordId].count(oldTopic) == 1) << "cannot remove topic that doesn't exist";
    word_topic_vals_[wordId][oldTopic]--;
    if(word_topic_vals_[wordId][oldTopic] == (int16_t)0)
      word_topic_vals_[wordId].erase(oldTopic);
    word_topic_delta_[wordId][oldTopic]--;
    if(word_topic_delta_[wordId][oldTopic] == (int16_t)0)
      word_topic_delta_[wordId].erase(oldTopic);
    s_total_ -= s_vector_[oldTopic];
    s_vector_[oldTopic] = (alpha_ * beta_) / (((real_t)summary_vals_[oldTopic]) + beta_sum_);
    s_total_ += s_vector_[oldTopic];
    //local cache
    docTopicVector[oldTopic]--;
    //docTopics: constant
    //numDocTopics: constant
    real_t denom = ((real_t)summary_vals_[oldTopic]) + beta_sum_;
    q_coeffs[oldTopic] = (alpha_ + ((real_t)docTopicVector[oldTopic])) / denom;
    r_total -= r_vector[oldTopic];
    r_vector[oldTopic] = (beta_ * ((real_t)docTopicVector[oldTopic])) / denom;
    r_total += r_vector[oldTopic];

    //build the q vector
    real_t q_total = 0.;
    std::vector<real_t> q_vector(K_,0.);
    std::vector<int32_t> wordTopics;      
    typedef std::map<int32_t,int16_t>::iterator IterType;
    for(IterType iter = word_topic_vals_[wordId].begin(); iter != word_topic_vals_[wordId].end(); ++iter)
    {
      int32_t topic = iter->first;
      int16_t count = iter->second;
      CHECK(count > 0) << "found non-positive word-topic val " << count;
      wordTopics.push_back(topic);
      q_vector[topic] = q_coeffs[topic] * ((real_t)count);
      q_total += q_vector[topic];
    }
    int numWordTopics = (int)wordTopics.size();

    //slow checks: on demand
    if(false)
    {
      real_t r_check = 0;
      real_t s_check = 0;
      real_t q_check = 0;
      for(int i=0;i<(int)r_vector.size();i++)
      {
        r_check += r_vector[i];
        CHECK(r_vector[i] >= 0) << "found negative r entry";
      }
      for(int i=0;i<(int)s_vector_.size();i++)
      {
        s_check += s_vector_[i];
        CHECK(s_vector_[i] >= 0) << "found negative s entry";
      }
      for(int i=0;i<(int)q_vector.size();i++)
      {
        q_check += q_vector[i];
        CHECK(q_coeffs[i] >= 0) << "found negative q entry";
        CHECK(q_vector[i] >= 0) << "found negative q entry";
      } 
      real_t rRatio = (r_check - r_total);
      if(rRatio < 0)
        rRatio = -rRatio;
      rRatio = rRatio / r_total;
      real_t sRatio = (s_check - s_total_);
      if(sRatio < 0)
        sRatio = -sRatio;
      sRatio = sRatio / s_total_;
      real_t qRatio = (q_check - q_total);
      if(qRatio < 0)
        qRatio = -qRatio;
      qRatio = qRatio / q_total;
      if(rRatio >= 0.00001 || sRatio >= 0.00001 || qRatio >= 0.00001)
      {
        LOG(INFO) << "b12 " << numWordTopics << " " << wordTopics.size() << " " << numDocTopics << " " << docTopics.size() << " " << q_vector.size() << " " << q_total << " " << r_total << " " << s_total_ << " " << r_check << " " << s_check << " " << oldTopic;
        CHECK(rRatio < 0.00001) << "r has become inconsistent";
        CHECK(sRatio < 0.00001) << "s has become inconsistent";
        CHECK(qRatio < 0.00001) << "q has become inconsistent";
      }
      CHECK(numDocTopics == (int)docTopics.size()) << "docTopics of incorrect size";
      CHECK(numDocTopics == (int)docTopicSet.size()) << "docTopicSet of incorrect size";
      CHECK(numWordTopics == (int)wordTopics.size()) << "wordTopics of incorrecrt size";
      for(int i=0;i<K_;i++)
      {
        if(docTopicSet.count(i) == 0)
        {
          CHECK(docTopicVector[i] == 0) << "docTopicVector has hidden nonzero";
          CHECK(r_vector[i] == 0) << "r_vector has hidden nonzero";
        }
        if(std::find(wordTopics.begin(), wordTopics.end(), i) == wordTopics.end())
        {
          CHECK(q_vector[i] == 0) << "q_vector has hidden nonzero";
        }
      }
      for(int i=0;i<numDocTopics;i++)
        CHECK(docTopicSet.count(docTopics[i]) == 1) << "docTopic Set and docTopics are inconsistent";
    }

    //sample
    real_t total_mass = q_total + r_total + s_total_;
    real_t sample = (*zero_one_rng_)() * total_mass;
    int32_t newTopic = -1;

    if (sample < q_total) 
    {
      qCounter += ((double)total_mass) / ((double)q_total);
      // The dart falls in [q_sum_ interval], which consists of [large_q_term |  
      // ... | small_q_term]. ~90% should fall in the q bucket.
      for (int i = 0; i < numWordTopics; ++i) 
      {
        sample -= q_vector[wordTopics[i]];
        if (sample <= 0.) 
        {
          newTopic = wordTopics[i];
          break;
        }
      }
      if(newTopic == -1)
      {
        // Overflow.
        LOG(INFO) << "sample = " << sample << " has overflowed in q bucket. Counters are " << qCounter << " " << rCounter << " " << sCounter;
        newTopic = wordTopics[numWordTopics - 1];
      }
    } 
    else 
    {
      //r bucket
      sample -= q_total;
      if (sample < r_total) 
      {
        rCounter += ((double)total_mass) / ((double)r_total);
        for (int i = 0; i < numDocTopics; ++i) 
        {
          sample -= r_vector[docTopics[i]];
          if (sample <= 0.) 
          {
            newTopic = docTopics[i];
            break;
          }
        }
        if(newTopic == -1)
        {
          // Overflow.
          LOG(INFO) << "sample = " << sample << " has overflowed in r bucket. Counters are " << qCounter << " " << rCounter << " " << sCounter;
          newTopic = docTopics[numDocTopics - 1];
        }
      } 
      else 
      {
        sCounter += ((double)total_mass) / ((double) s_total_);
        // s bucket.
        sample -= r_total;    
        for (int i = 0; i < K_; ++i) 
        {
          sample -= s_vector_[i];
          if (sample <= 0.) 
          {
            newTopic = i;
            break;
          }
        }
        if(newTopic == -1)
        {
          // Overflow.
          LOG(INFO) << "sample = " << sample << " has overflowed in s bucket. Counters are " << qCounter << " " << rCounter << " " << sCounter;
          newTopic = K_ - 1;
        }
      }
    }
    if(newTopic == -1)
    {
      newTopic = K_ - 1;
      LOG(INFO) << "global overflow. Counters are " << qCounter << " " << rCounter << " " << sCounter;
    }

    //Add new variable assignment by updating the cache
    //global cache
    summary_vals_[newTopic]++;
    word_topic_vals_[wordId][newTopic]++;
    word_topic_delta_[wordId][newTopic]++;
    if(word_topic_delta_[wordId][newTopic] == (int16_t)0)
      word_topic_delta_[wordId].erase(newTopic);
    s_total_ -= s_vector_[newTopic];
    s_vector_[newTopic] = (alpha_ * beta_) / (((real_t)summary_vals_[newTopic]) + beta_sum_);
    s_total_ += s_vector_[newTopic];
    //local cache
    docTopicVector[newTopic]++;
    if(docTopicSet.count(newTopic) == 0)
    {
      docTopicSet.insert(newTopic);
      docTopics.push_back(newTopic);
      numDocTopics++;
    }
    denom = ((real_t)summary_vals_[newTopic]) + beta_sum_;
    q_coeffs[newTopic] = (alpha_ + ((real_t)docTopicVector[newTopic])) / denom;
    r_total -= r_vector[newTopic];
    r_vector[newTopic] = (beta_ * ((real_t)docTopicVector[newTopic])) / denom;
    r_total += r_vector[newTopic];

    // Finally, update the topic assignment z in doc, and update word-topic
    // table and summary row.
    if (oldTopic != newTopic) {
      it.SetTopic(newTopic);
    }
  }
}

// ====================== georgs addition =========================
void FastDocSamplerVirtual::buildGlobalCache() 
{
  //word-topic table and delta
  word_topic_vals_.clear();
  word_topic_delta_.clear();
  std::map<int32_t,int16_t> emptyRow;
  for(int32_t wordId = 0;wordId < V_;wordId++)
  {
    std::map<int32_t,int16_t> row;
    petuum::RowAccessor word_topic_row_acc;
    const auto & word_topic_row = word_topic_table_.Get<petuum::SortedVectorMapRow<int32_t> >(wordId, &word_topic_row_acc);
    std::vector<petuum::Entry<int32_t> > word_topic_row_buff;
    word_topic_row_buff.resize(K_);
    word_topic_row.CopyToVector(&word_topic_row_buff);
    STATS_APP_DEFINED_ACCUM_VAL_INC(word_topic_row_buff.size());
    for (auto & wt_it : word_topic_row_buff) 
    {
      int32_t topic = wt_it.first;
      int32_t count = wt_it.second;
      CHECK(count > 0) << "found non-positive word count";
      row[topic] = (int16_t)count;
    }
    word_topic_vals_.push_back(row);
    word_topic_delta_.push_back(emptyRow);
  }

  //summary row
  summary_vals_.clear();
  for(int i=0;i<K_;i++)
  {
    summary_vals_.push_back(0);
    for(int j=0;j<V_;j++)
    {
      if(word_topic_vals_[j].count(i) == 1)
      {
        summary_vals_[i] += (int32_t)word_topic_vals_[j][i];
      }
    }
  }

  //s bucket
  s_vector_.clear();
  s_total_ = 0;
  for(int i=0;i<K_;i++)
  {
    real_t val = (alpha_ * beta_) / (((real_t)summary_vals_[i]) + beta_sum_);
    s_vector_.push_back(val);
    s_total_ += val;
  } 
}


void FastDocSamplerVirtual::pushRow(int32_t index)
{
  int numUpdates = (int)word_topic_delta_[index].size();
  std::map<int32_t,int16_t> row = word_topic_delta_[index];
  petuum::UpdateBatch<int32_t> word_topic_updates(numUpdates);
  typedef std::map<int32_t,int16_t>::iterator IterType;
  int32_t counter = 0;
  for( IterType iter = row.begin() ; iter != row.end() ; ++iter )
  {
    word_topic_updates.UpdateSet(counter, iter->first, (int32_t)iter->second);
    counter++;
  }
  word_topic_table_.BatchInc(index, word_topic_updates);
  word_topic_delta_[index].clear();
}

void FastDocSamplerVirtual::pullRow(int32_t index, std::map<int32_t,int16_t> * changes)
{
  std::map<int32_t, int16_t> oldRow = word_topic_vals_[index];
  std::map<int32_t, int16_t> newRow;

  //fetch row from parameter server
  petuum::RowAccessor word_topic_row_acc;
  const auto & word_topic_row = word_topic_table_.Get<petuum::SortedVectorMapRow<int32_t> >(index, &word_topic_row_acc);
  std::vector<petuum::Entry<int32_t> > word_topic_row_buff;
  word_topic_row_buff.resize(K_);
  word_topic_row.CopyToVector(&word_topic_row_buff);
  STATS_APP_DEFINED_ACCUM_VAL_INC(word_topic_row_buff.size());
  for (auto & wt_it : word_topic_row_buff) 
  {
    int32_t topic = wt_it.first;
    int16_t count = (int16_t)wt_it.second;
    CHECK(count > 0) << "found non-positive word count";
    newRow[topic] = count;
  }

  //build error strings
  std::stringstream oldRowStream;
  typedef std::map<int32_t,int16_t>::iterator IterType;
  for( IterType iter = oldRow.begin() ; iter != oldRow.end() ; ++iter )
    oldRowStream << iter->first << ":" << iter->second << " ";
  std::stringstream newRowStream;
  for( IterType iter = newRow.begin() ; iter != newRow.end() ; ++iter )
    newRowStream << iter->first << ":" << iter->second << " ";

  //check row consistency
  if(FLAGS_num_table_threads == 1 && FLAGS_num_comm_channels_per_client == 1 && FLAGS_client_id == 0 && thread_id == 0)
    CHECK(oldRow == newRow) << "row changes in background, old row: " << oldRowStream.str() << " new row: " << newRowStream.str();

  //record topic changes
  for( IterType iter = oldRow.begin() ; iter != oldRow.end() ; ++iter )
  {
    int32_t topic = iter->first;
    int16_t count = iter->second;
    CHECK(count > 0) << "found non-positive count in old row " << count << " newrow " << newRowStream.str() << " oldrow size " << oldRowStream.str();
    if(changes->count(topic) == 0)
      changes->insert(std::pair<int32_t,int16_t>(topic, 0));
    changes->at(topic) -= count;
  }
  for( IterType iter = newRow.begin() ; iter != newRow.end() ; ++iter )
  {
    int32_t topic = iter->first;
    int16_t count = iter->second;
    CHECK(count > 0) << "found non-positive count in new row " << count << " newrow " << newRowStream.str() << " oldrow size " << oldRowStream.str();
    if(changes->count(topic) == 0)
      changes->insert(std::pair<int32_t,int16_t>(topic, 0));
    changes->at(topic) += count;
  }

  //update the cache
  word_topic_vals_[index] = newRow;
}


void FastDocSamplerVirtual::pushMany(std::vector<int32_t> indeces)
{
  int indexSize = (int)indeces.size();
  for(int i=0;i<indexSize;i++)
  {
    pushRow(i);
  }
}

void FastDocSamplerVirtual::pullMany(std::vector<int32_t> indeces)
{
  int indexSize = (int)indeces.size();
  std::map<int32_t,int16_t> changes;
  for(int i=0;i<indexSize;i++)
  {
    pullRow(i, &changes);
  }
  
  //summary row
  typedef std::map<int32_t,int16_t>::iterator IterType;
  for( IterType iter = changes.begin() ; iter != changes.end() ; ++iter )
  {
    summary_vals_[iter->first] += (int32_t)iter->second;
  }

  //s bucket
  s_vector_.clear();
  s_total_ = 0;
  for(int i=0;i<K_;i++)
  {
    real_t val = (alpha_ * beta_) / (summary_vals_[i] + beta_sum_);
    s_vector_.push_back(val);
    s_total_ += val;
  }
}

void FastDocSamplerVirtual::push(RowUpdateItem item)
{
  std::vector<int32_t> updateSchedule;
  int currentRow = item.first;
  if(!FLAGS_is_local_sync)
  {
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == V_)
        currentRow = 0;
      updateSchedule.push_back(currentRow);
      currentRow++;
    }

    if(FLAGS_is_bipartite)
    {
      for(int i=0; i<V_;i++)
      {
        if((i+FLAGS_client_id) % 2 == 1)
          continue;
        updateSchedule.push_back(i);
      }
    }
  }
  else
  {
    CHECK(false) << "not implemented";
    /*for (int i = 0; i < totalRows; i++) 
    {
        pushRow(i, ((FLAGS_client_id % 2 == 1) ? true : false ));
    }
    
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == totalRows)
        currentRow = 0;
      pushOther(currentRow, ((FLAGS_client_id % 2 == 0) ? true : false ));
      currentRow++;
    }*/
  }
  
  pushMany(updateSchedule);
}

void FastDocSamplerVirtual::pull(RowUpdateItem item)
{
  std::vector<int32_t> updateSchedule;
  int currentRow = item.first;
  if(!FLAGS_is_local_sync)
  {
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == V_)
        currentRow = 0;
      updateSchedule.push_back(currentRow);
      currentRow++;
    }

    if(FLAGS_is_bipartite)
    {
      for(int i=0; i<V_;i++)
      {
        if((i+FLAGS_client_id) % 2 == 1)
          continue;
        updateSchedule.push_back(i);
      }
    }
  }
  else
  {
    CHECK(false) << "not implemented";
    /*for (int i = 0; i < totalRows; i++) 
    {
        pushRow(i, ((FLAGS_client_id % 2 == 1) ? true : false ));
    }
    
    for (int i = 0; i < item.numRows; ++i) 
    {
      if(currentRow == totalRows)
        currentRow = 0;
      pushOther(currentRow, ((FLAGS_client_id % 2 == 0) ? true : false ));
      currentRow++;
    }*/
  }
  
  pullMany(updateSchedule);
}


std::map<int32_t,int16_t> FastDocSamplerVirtual::get_word_topic_row(int wordId)
{
  return word_topic_vals_[wordId];
}

std::vector<int32_t> FastDocSamplerVirtual::get_summary_vals()
{
  return summary_vals_;
}




























}  // namespace lda
