client_id=0

while [ $client_id -le 127 ]; do

    data_file=/usr0/home/gschoenh/BigData/nytimes/nytimes.0.01.128.${client_id}
    output_file=/usr0/home/gschoenh/BigData/nytimes/nytimes.proc.0.01.128.${client_id}
    num_partitions=1

    cmd="GLOG_logtostderr=true \
./../bin/data_preprocessor \
--data_file=$data_file  \
--output_file=$output_file \
--num_partitions=$num_partitions"

    eval $cmd
    echo $cmd
    client_id=$(( client_id+1 ))
done
