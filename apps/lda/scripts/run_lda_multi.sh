#!/bin/bash -u

# Find other Petuum paths by using the script's path
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_root=`dirname $script_dir`

doc_filename="/usr0/home/gschoenh/BigData/nytimes/nytimes123"
doc_file_suffix="bla"
num_vocabs=200001
max_vocab_id=200000
num_topics=100000000
alpha=10
beta=10
communication_factor=-2
virtual_staleness=-2
is_bipartite=false
is_local_sync=false
num_work_units=-2
compute_ll_interval=-2
num_iters_per_work_unit=10
num_clocks_per_work_unit=10
error_threshold=0

word_topic_table_process_cache_capacity=$(( max_vocab_id+1 ))

output_path=${app_root}"/output/empty"
parm_file=/usr0/home/gschoenh/Dropbox/Work/Projects/ChallengingNetworks/parmFilesLDA/empty.txt
system_path=${app_root}/

table_staleness=-8
consistency_model=SomeBullshit
client_bandwidth_mbps=33
server_bandwidth_mbps=44
num_comm_channels_per_client=19
num_table_threads=10
host_filename="../../../machinefiles/localserver"

bg_idle_milli=2
server_idle_milli=2
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

doc_file=$(readlink -f $doc_filename)
host_file=$(readlink -f $host_filename)
progname=lda_multi
prog_path=$app_root/bin/$progname
log_dir=$app_root/log/log

# Parse hostfile
host_list=`cat $host_file | awk '{ print $2 }'`
unique_host_list=`cat $host_file | awk '{ print $2 }' | uniq`
num_unique_hosts=`cat $host_file | awk '{ print $2 }' | uniq | wc -l`
host_list=`cat $host_file | awk '{ print $2 }'`
num_hosts=`cat $host_file | awk '{ print $2 }' | wc -l`

ssh_options="-oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null -oLogLevel=quiet"

# Kill previous instances of this program
echo "Killing previous instances of '$progname' on servers, please wait..."
for ip in $unique_host_list; do
  ssh $ssh_options $ip \
    killall -q $progname
done
echo "All done!"
# exit

# Spawn program instances
client_id=0
for ip in $host_list; do
  echo Running master process on $ip
  log_path=${log_dir}.${client_id}
  numa_index=$(( client_id%num_unique_hosts ))
#"ls /l0;
  cmd="rm -rf ${log_path}; mkdir -p ${log_path};
GLOG_logtostderr=true \
      GLOG_log_dir=${log_path} \
      GLOG_v=-1 \
      GLOG_minloglevel=0 \
      GLOG_vmodule="" \
      $prog_path \
    --num_clients $num_hosts \
    --num_comm_channels_per_client $num_comm_channels_per_client \
    --init_thread_access_table=false \
    --num_table_threads ${num_table_threads} \
    --client_id $client_id \
    --hostfile ${host_file} \
    --consistency_model $consistency_model \
    --client_bandwidth_mbps $client_bandwidth_mbps \
    --server_bandwidth_mbps $server_bandwidth_mbps \
    --bg_idle_milli $bg_idle_milli \
    --thread_oplog_batch_size $thread_oplog_batch_size \
    --row_candidate_factor ${row_candidate_factor} \
    --server_idle_milli $server_idle_milli \
    --update_sort_policy $update_sort_policy \
    --numa_opt=${numa_opt} \
    --numa_index ${numa_index} \
    --numa_policy ${numa_policy} \
    --naive_table_oplog_meta=${naive_table_oplog_meta} \
    --suppression_on=${suppression_on} \
    --use_approx_sort=${use_approx_sort} \
    --table_staleness $table_staleness \
    --row_type 0 \
    --row_oplog_type ${row_oplog_type} \
    --nooplog_dense_serialized \
    --oplog_type ${oplog_type} \
    --append_only_oplog_type DenseBatchInc \
    --append_only_buffer_capacity ${append_only_buffer_capacity} \
    --append_only_buffer_pool_size ${append_only_buffer_pool_size} \
    --bg_apply_append_oplog_freq ${bg_apply_append_oplog_freq} \
    --process_storage_type ${process_storage_type} \
    --no_oplog_replay=${no_oplog_replay} \
    --server_push_row_upper_bound ${server_push_row_upper_bound} \
    --client_send_oplog_upper_bound ${client_send_oplog_upper_bound} \
    --alpha $alpha \
    --beta $beta \
    --num_topics $num_topics \
    --num_vocabs $num_vocabs \
    --max_vocab_id $max_vocab_id \
    --num_clocks_per_work_unit $num_clocks_per_work_unit \
    --num_iters_per_work_unit $num_iters_per_work_unit \
    --word_topic_table_process_cache_capacity $word_topic_table_process_cache_capacity \
    --num_work_units $num_work_units \
    --compute_ll_interval=$compute_ll_interval \
    --doc_file=${doc_file} \
    --communication_factor=$communication_factor \
    --virtual_staleness=$virtual_staleness \
    --is_bipartite=$is_bipartite \
    --is_local_sync=$is_local_sync \
    --doc_file_suffix=$doc_file_suffix \
    --error_threshold=$error_threshold \
    --output_path=$output_path \
    --parm_file=$parm_file \
    --system_path=$system_path"

#  echo $cmd
  ssh $ssh_options $ip $cmd &

# Wait a few seconds for the name node (client 0) to set up
  if [ $client_id -eq 0 ]; then
    echo "Waiting for name node to set up..."
    sleep 3
  fi
  client_id=$(( client_id+1 ))
  exit
done
