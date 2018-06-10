#!/usr/bin/env python
 
import sys
from launch_utils import launch_util

#hostfile = "machinefiles/local"
hostfile = "machinefiles/1node"
progfile = "debug2/KMeansExample"

params = {
"load_injection" : 10000,
   "threshold" : 0.9,
   "rapid_reassgn_group_mode" : 2, # 0, 1:with "50+x", 2: with "50+x, 4050-x"
   "rapid_reassgn_mode" : 1, # :OFF, 1:ON
  "kStaleness" : 1,
  "kModelType" : "SSP",  # {ASP/SSP/BSP}
  #"num_dims" : 123,
  #"num_dims" : 18,
  #"num_dims" : 784,
  #"num_dims" : 54686452,
  "num_dims" : 16609143,
  "num_workers_per_node" : 10, # 3
  "num_servers_per_node" : 10,
  "num_iters" : 5, #1000
  "kStorageType" : "Vector",  # {Vector/Map}
  "hdfs_namenode" : "proj10",
  "hdfs_namenode_port" : 9000,
  #"input" : "hdfs:///datasets/classification/a9",
  #"input" : "hdfs:///jasper/SUSY",
  #"input" : "hdfs:///jasper/mnist8m",
  #"input" : "hdfs:///datasets/classification/kdd12",
  "input" : "hdfs:///datasets/classification/webspam",
  #"K" : 2,
  #"K" : 10,
  "K" : 2,
  "batch_size" : 10, # 100
  "alpha" : 0.01,
  "kmeans_init_mode" : "random", # random/kmeans++/kmeans_parallel
  "report_interval" : "1",
  }

env_params = (
  "GLOG_logtostderr=true "
  "GLOG_v=-1 "
  "GLOG_minloglevel=0 "
  "LIBHDFS3_CONF=/data/opt/course/hadoop/etc/hadoop/hdfs-site.xml"
  )

launch_util(progfile, hostfile, env_params, params, sys.argv)
