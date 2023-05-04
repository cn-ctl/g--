#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

cd $(dirname $0)

echo "Making(Updating) irsim"
make -C irsim

RUN=$1
EXTEND_TYPE=$2
CONTINUE=$3

EXTEND_TYPES=(extend1 extend2 both)

mkdir -p ./workdir

if ! [ -z $RUN ]
then
  rm -f ./workdir/saved_binary.sh
fi

if [ -e ./workdir/saved_binary.sh ]
then
  source ./workdir/saved_binary.sh
fi

help(){
  echo "Usage: $0 path_to_parser_binary extend_type"
  echo "Support extend_types: ${EXTEND_TYPES[*]}"
  rm -f ./workdir/saved_binary.sh
  exit 1
}

if [ -z $RUN ]
then
  help
fi

if echo ${EXTEND_TYPES[*]} | grep $EXTEND_TYPE -w -q; then
  true;
else
  help
fi

if ! [ -x $RUN ]
then
  echo "Error: file \"$RUN\" is not executable"
  rm -f ./workdir/saved_binary.sh
  exit 1
fi

echo "RUN=$RUN" > ./workdir/saved_binary.sh
echo "EXTEND_TYPE=$EXTEND_TYPE" >> ./workdir/saved_binary.sh
echo "CONTINUE=$CONTINUE" >> ./workdir/saved_binary.sh

report_error(){
  echo -e "${RED}${BOLD} $dir test [$(basename $fcmm)]" "$1" "${NC}${NORMAL}"
  if [ -z $CONTINUE ]
  then
    read -p "Enter [c] to continue, or [Enter] to abort: " txt
    if [ -z "$txt" ] || [ $txt != 'c' ]
    then
      exit 1
    fi
  fi
}

if timeout --help > /dev/null 2>&1; then #if has `timeout` command
  PREFIX="timeout 10 ";
else
  echo "timeout command is not support in current environment"
  echo "running time will not be counted"
  PREFIX="";
fi;

base=(sample normal1 normal2 normal3 handmade advance1 advance2)
extend1=(extend1)
extend2=(extend2)
extendboth=(extendboth)

run(){
  cp $1 ./workdir/a.cmm
  cp ${1%.cmm}.json ./workdir/a.json
  rm -f ./workdir/a.ir
  
  let total=$total+1

  if $PREFIX $RUN ./workdir/a.cmm ./workdir/a.ir 2>&1 > ./workdir/a.out; then
    true; #do nothing
  else
    report_error "RE or TLE"
    continue
  fi
}

test_group(){
  for dir in $@; do
    for fcmm in ./tests/$dir/*.cmm; do
      run $fcmm

      if ! [ -e ./workdir/a.ir ]
      then
        report_error "Should translate"
        continue
      fi;

      if $PREFIX python3 ./check.py; then
        echo $dir test [$(basename $fcmm)] matched
        let passed=$passed+1
      else
        report_error "mismatch or TLE"
      fi
    done
  done
}

test_fault(){
  for dir in $@; do
    for fcmm in ./tests/$dir/*.cmm; do
      run $fcmm

      if [ -e ./workdir/a.ir ]
      then
        report_error "Should not translate"
      else
        echo $dir test [$(basename $fcmm)] passed
        let passed=$passed+1
      fi
    done
  done
}

echo -n 0 > workdir/count

passed=0
total=0

test_group ${base[*]}

if [ $EXTEND_TYPE != extend2 ]
then
  test_group ${extend1[*]}
else
  test_fault ${extend1[*]}
fi

if [ $EXTEND_TYPE != extend1 ]
then
  test_group ${extend2[*]}
else
  test_fault ${extend2[*]}
fi

if [ $EXTEND_TYPE = both ]
then
  test_group ${extendboth[*]}
else
  test_fault ${extendboth[*]}
fi

echo PASSED $passed/$total

echo -n "irsim executes about "
cat workdir/count
echo " instructions"
