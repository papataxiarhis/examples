#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage $0 <username> <nodes_list>     (use ',' to separate nodes in list )"
	exit
fi

set -x
nodes=$(echo $2 | tr "," "\n")
user=$1

if [ 1 -eq 1 ]; then

    for node in $nodes
    do
        rsync -avz  --exclude=.git --exclude '*.o' --exclude '*.h' --exclude '*.c' --exclude '*.pyc' --exclude examples/* --exclude agent_modules/*  --exclude .repo/ ../../  --exclude hidden_repo/ -e ssh $user@$node:~/wishful-github-manifest/
        rsync -avz  ../../agent_modules/iperf/  -e ssh $user@$node:~/wishful-github-manifest/agent_modules/iperf/
        rsync -avz  ../../agent_modules/net_linux/  -e ssh $user@$node:~/wishful-github-manifest/agent_modules/net_linux/
        rsync -avz  ../../controller_modules/wireless_topology/  -e ssh $user@$node:~/wishful-github-manifest/controller_modules/wireless_topology/
        rsync -avz  ../../agent_modules/module_lte/  -e ssh $user@$node:~/wishful-github-manifest/agent_modules/module_lte/
        rsync -avz  ../../examples/lte_everwishful/  -e ssh $user@$node:~/wishful-github-manifest/examples/lte_everwishful/
    done

fi

set +x



