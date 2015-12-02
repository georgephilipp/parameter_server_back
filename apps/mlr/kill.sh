#!/bin/bash -u

# Init weights
use_weight_file=false
weight_file=/tank/projects/biglearning/jinlianw/data/mlr_data/imagenet_llc.weight


# Data parameters:
num_train_data=0  # 0 to use all training data.

# Covtype
train_file=imnet
#train_file_path=/tank/projects/biglearning/jinlianw/parameter_server.git/apps/mlr/datasets/covtype.scale.train.small
#test_file_path=/tank/projects/biglearning/jinlianw/parameter_server.git/apps/mlr/datasets/covtype.scale.test.small
#train_file_path=/tank/projects/biglearning/jinlianw/data/mlr_data/imagenet_llc/imnet.train.50.train
#test_file_path=/tank/projects/biglearning/jinlianw/data/mlr_data/imagenet_llc/imnet.train.10.test
train_file_path=/usr0/home/gschoenh/imnet
test_file_path=/usr0/home/gschoenh/imnet
global_data=true
perform_test=false

# Execution parameters:
num_epochs=100
num_batches_per_epoch=1
#learning_rate=1
learning_rate=1
decay_rate=0.99
num_epochs_per_eval=1
num_train_eval=1000   # large number to use all data.
num_test_eval=200
num_secs_per_checkpoint=200000

# System parameters:
#host_filename="../../machinefiles/servers.4"
host_filename="../../machinefiles/localserver"
consistency_model="SSPPush"
num_worker_threads=1
num_comm_channels_per_client=1
table_staleness=2
row_oplog_type=0

# SSPAggr parameters:
bg_idle_milli=2
# Total bandwidth: bandwidth_mbps * num_comm_channels_per_client * 2
client_bandwidth_mbps=540
server_bandwidth_mbps=540
# bandwidth / oplog_push_upper_bound should be > miliseconds.
thread_oplog_batch_size=21504000
server_idle_milli=2
update_sort_policy=Random
row_candidate_factor=5

append_only_buffer_capacity=$((1024*1024*4))
append_only_buffer_pool_size=3
bg_apply_append_oplog_freq=64

client_send_oplog_upper_bound=1000
server_push_row_upper_bound=500

oplog_type=Dense
process_storage_type=BoundedDense

no_oplog_replay=true
numa_opt=false
numa_policy=Even
naive_table_oplog_meta=false
suppression_on=false
use_approx_sort=false

# Figure out the paths.
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_dir=`dirname $script_dir`
progname=mlr_main
multiname=mlr_multi
prog_path=$app_dir/mlr/${progname}
host_file=$(readlink -f $host_filename)

ssh_options="-oStrictHostKeyChecking=no \
-oUserKnownHostsFile=/dev/null \
-oLogLevel=quiet"

# Parse hostfile
host_list=`cat $host_file | awk '{ print $2 }'`
unique_host_list=`cat $host_file | awk '{ print $2 }' | uniq`
num_unique_hosts=`cat $host_file | awk '{ print $2 }' | uniq | wc -l`
host_list=`cat $host_file | awk '{ print $2 }'`
num_hosts=`cat $host_file | awk '{ print $2 }' | wc -l`

output_dir="${app_dir}/output_feb_16_4x1_mbssp_debug"
output_dir="${output_dir}/mlr.${train_file}.S${table_staleness}.E${num_epochs}"
output_dir="${output_dir}.M${num_unique_hosts}"
output_dir="${output_dir}.T${num_worker_threads}"
output_dir="${output_dir}.B${num_batches_per_epoch}.${consistency_model}.${learning_rate}_full"

output_file_prefix=$output_dir/mlr_out  # prefix for program outputs
rm -rf ${output_dir}
mkdir -p ${output_dir}
echo Output Dir is $output_dir

log_dir=$output_dir/logs
stats_path=${output_dir}/mlr_stats.yaml

# Kill previous instances of this program
echo "Killing previous instances of '$progname' on servers, please wait..."
for ip in $unique_host_list; do
    echo "killing ".$ip
  ssh $ssh_options $ip \
    killall -q $progname
done
echo "Killing previous instances of '$multiname' on servers, please wait..."
for ip in $unique_host_list; do
    echo "killing ".$ip
  ssh $ssh_options $ip \
    killall -q $multiname
done
echo "All done!"
# exit

