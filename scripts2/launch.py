#!/usr/bin/env python

import sys
from launch_utils import launch_util

#hostfile = "machinefiles/local"
hostfile = "machinefiles/1node"
progfile = "debug2/BasicExample"

params = {
  "load_injection" : 10,
  "threshold" : 0.9, 
  "rapid_reassgn_group_mode" : 0, # 0, 1:with "50+x", 2: with "50+x, 4050-x"
  "rapid_reassgn_mode" : 0, # :OFF, 1:ON
  "kStaleness" : 1,
     "kModelType" : "SSP",  # {ASP/SSP/BSP/SparseSSP}
     #"num_dims" : 54686452,
     #"num_dims" : 123,
     #"num_dims" : 780,
     #"num_dims" : 18,
     "num_dims" : 16609143,
     "batch_size" : 10000,
     "num_workers_per_node" : 10,
     "num_servers_per_node" : 10,
     "num_iters" : 5,
     "kStorageType" : "Vector",  # {Vector/Map}
     "hdfs_namenode" : "proj10",
     "hdfs_namenode_port" : 9000,
     #"input" : "hdfs:///jasper/kdd12",
     #"input" : "hdfs:///datasets/classification/a9",
     #"input" : "hdfs:///jasper/mnist8m",
     #"input" : "hdfs:///jasper/SUSY",
     "input" : "hdfs:///datasets/classification/webspam",
     "alpha" : 0.1, # learning rate
     "learning_rate_decay" : 500,
     "report_interval" : 1,
     "trainer" : "lr",  # {lr/lasso/svm}
     "lambda" : 0.01,
     #"load_injection" : 10,
}

env_params = (
  "GLOG_logtostderr=true "
  "GLOG_v=-1 "
  "GLOG_minloglevel=0 "
  "LIBHDFS3_CONF=/data/opt/course/hadoop/etc/hadoop/hdfs-site.xml"
)

# use `python scripts/launch.py` to launch the distributed programs
# use `python scripts/launch.py kill` to kill the distributed programs
launch_util(progfile, hostfile, env_params, params, sys.argv)
