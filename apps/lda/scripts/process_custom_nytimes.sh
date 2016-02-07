client_id=0

while [ $client_id -le 0 ]; do

    data_file=/usr0/home/gschoenh/BigData/nytimes/nytimes.1.2.1
    output_file=/usr0/home/gschoenh/BigData/nytimes/nytimes.proc.1.2.1
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
