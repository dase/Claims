#!/bin/sh
set -e
cd $CLAIMS_HOME/sbin/2-claims-conf/
source ./load-config.sh

##################
# start test     #
##################
  cd $CLAIMS_HOME
  cd install
  ulimit -c unlimited

if [ "$1" = "" ]; then
  runloops=1
else
  runloops=$1
fi

if [ "$2" = "" ]; then
  concurrency_count=1
else
  concurrency_count=$2
fi

if [ "$3" = "" ]; then
 echo "please input test case as the third param."
 exit 1
fi

echo "runloops=[$runloops]"
echo "concurrency_count=[$concurrency_count]"

thislog=$logfilepath/client.$(date +%Y-%m-%d).log

testmode="userresult"

if [ "$testmode" = "userresult" ]; then
cd $CLAIMS_HOME/sbin/claims-test
for((loop=1;loop<=runloops;loop++))
do
{
 for((cur=1;cur<=concurrency_count;cur++))  
 do
 {
  datestr=`date '+%Y-%m-%d %H:%M:%S'`
  thisstartstr="========run test:[$3] [$loop-$cur] time: $datestr========"
  echo -e "\033[33m$thisstartstr\033[0m"
#  echo $thisstartstr >> $thislog
  cd $CLAIMS_HOME/sbin/claims-test/python 
  python $3.py > $CLAIMS_HOME/sbin/claims-test/testresult/$3-$cur-py.result
  sleep 1

 }&
 done
 wait
}
done
else
for((loop=1;loop<=runloops;loop++))
do
{
 for((cur=1;cur<=concurrency_count;cur++))  
 do
 {

  datestr=`date '+%Y-%m-%d %H:%M:%S'`
  thisstartstr="========run client [$loop-$cur] time: $datestr========"
  echo -e "\033[33m$thisstartstr\033[0m"
  echo $thisstartstr >> $thislog
  cd $CLAIMS_HOME/sbin/claims-test/python 
  python $3.py>> $thislog
  sleep 1

 }&
 done
 wait
}
done
fi
