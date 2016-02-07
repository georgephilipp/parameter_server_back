#!/usr/bin/env bash

table_staleness=0
consistency_model=SSPPush

num_work_units=4

#dataset=nytimes
#dataset=20news
#dataset=culp_e
num_comm_channels_per_client=1

num_worker_threads=1

num_topics=100

communication_factor=-1
virtual_staleness=-1
is_bipartite=false
is_local_sync=false

num_clocks_per_work_unit=1
num_iters_per_work_unit=1

bg_idle_milli=2
server_idle_milli=2
client_bandwidth_mbps=0
server_bandwidth_mbps=0
thread_oplog_batch_size=14000
row_candidate_factor=100
update_sort_policy=RelativeMagnitude
numa_opt=false
numa_policy=Even

row_oplog_type=0

server_push_row_upper_bound=500
client_send_oplog_upper_bound=5000

append_only_buffer_capacity=$((1024*512))
append_only_buffer_pool_size=3
bg_apply_append_oplog_freq=-1

process_storage_type=BoundedDense
oplog_type=Dense
no_oplog_replay=true
naive_table_oplog_meta=false
suppression_on=false
use_approx_sort=false

app_output_dir=${app_root}/output/nytimes_${consistency_model}_${num_work_units}
app_output_dir=${app_output_dir}_${client_bandwidth_mbps}
app_output_dir=${app_output_dir}_${update_sort_policy}_${server_push_row_upper_bound}
app_output_dir=${app_output_dir}_${client_send_oplog_upper_bound}_${num_comm_channels_per_client}_${num_clocks_per_work_unit}_16_${thread_oplog_batch_size}

if [ -z ${iskill+x} ]; then
    echo "kill"
else
    rm -rf ${app_output_dir}
    mkdir ${app_output_dir}
    cp ${app_root}/scripts/nytimes.config.sh ${app_output_dir}
fi


#doc_filename="datasets/processed/20news"
#doc_filename="/home/jinliang/data/lda_data/processed/20news.1"
doc_filename="/usr0/home/gschoenh/BigData/nytimes/nytimes.proc.0.01.1"
#doc_filename="/l0/data/pubmed_8"
#doc_filename="../../../data/lda_data/processed/EO_1_1.1"

output_file_prefix=${app_output_dir}/nytimes

#num_vocabs=141043 # pubmed
#max_vocab_id=141042 # pubmed
num_vocabs=101636 # nytimes
max_vocab_id=101635  # nytimes
#num_vocabs=53974 # 20news
#max_vocab_id=53974 # 20news

#num_vocabs=160024
#max_vocab_id=160024

#num_vocabs=400 #culp_eo
#max_vocab_id=400 #culp_eo

word_topic_table_process_cache_capacity=$(( max_vocab_id+1 ))

host_filename="../../../machinefiles/localserver"
#host_filename="../../machinefiles/servers.mylda.ssp"
#host_filename="../../machinefiles/servers.mylda.64"

compute_ll_interval=1

stats_path=${app_output_dir}/lda_stats.yaml

log_dir=${app_output_dir}"/logs.lda.s"$table_staleness
log_dir="${log_dir}.C${consistency_model}"
log_dir="${log_dir}.BW${client_bandwidth_mbps}"

multiname=lda_multi
progname=lda_main

alpha=0.1
beta=0.1
