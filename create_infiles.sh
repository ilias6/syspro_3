#!/bin/bash

reverseDate(){
  for file in ${files[*]}
  do
    day=${file:8:2}
    month=${file:5:2}
    year=${file:0:4}
    newName="$day-$month-$year"
    mv -v ./$file ./$newName >> /dev/null
  done
}

makeRecords(){
  files=($(ls -X))
  j=0
  for file in ${files[*]}
  do
    numOfLines=$(wc -l $file | cut -d ' ' -f1)
    for ((i=numOfLines; i < numofRec; ++i))
    do
      #FOR ENTER
      len=$(($RANDOM%5+1))
      str=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $len | head -n 1)
      recID=$id$str
      len=$(($RANDOM%9+3))
      first=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $len | head -n 1)
      last=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $len | head -n 1)
      len=${#diseases[*]}
      index=$(($RANDOM%$len))
      disID=${diseases[$index]}
      age=$(($RANDOM%120 +1))
      str="ENTER"
      echo "$recID $str $first $last $disID $age" >> $file
      #FOR EXIT
      str="EXIT"
      len=${#files[*]}
      p=$(($RANDOM%6))
      if [ $p -gt 0 ]
      then
        n=$(($RANDOM%($len-$j)))
        index=$(($n+$j))
        if [ $index -eq $j ] && [ $i -lt $(($numofRec-1)) ]
        then
          echo "$recID $str $first $last $disID $age" >> ${files[$index]}
          i=$(($i+1))
        else
          numOfLines=$(wc -l ${files[$index]} | cut -d ' ' -f1)
          if [ $numOfLines -lt $numofRec ]
          then
            echo "$recID $str $first $last $disID $age" >> ${files[$index]}
          fi
        fi
      fi
      id=$(($id+1))
    done
    j=$(($j+1))
  done
}

makeFiles(){
  for ((i=0; i < numofFiles; ++i))
  do
    n=$RANDOM
    day=$(($n%30+1))
    month=$(($n%12+1))
    year=$(($n%32+1990))

    if [ $day -lt 10 ]
    then
      if [ $month -lt 10 ]
      then
        touch "$year-0$month-0$day"
      else
        touch "$year-$month-0$day"
      fi
    elif [ $month -lt 10 ]
    then
      touch "$year-0$month-$day"
    else
      touch "$year-$month-$day"
    fi
  done
}

if [ $# -ne 5 ]
then
  echo "Invalid number of arguments"
  exit 1
fi

if [ ! -e $1 ]
then
  echo "File of diseases does not exist!"
  exit 1
fi

if [ ! -e $2 ]
then
  echo "File of countries does not exist!"
  exit 1
fi

dirName=$3
numofFiles=$4
numofRec=$5

if [ $4 -lt 0 ] || [ $5 -lt 0 ]
then
  echo "Please give non-negative integers"
  exit 1
fi

i=0
while read line
do
  diseases[$i]=$line
  i=$(($i+1))
done < $1

i=0
while read line
do
  countries[$i]=$line
  i=$(($i+1))
done < $2


mkdir ./$dirName
cd ./$dirName
id=0
for country in ${countries[*]}
do
  mkdir ./$country
  cd ./$country
  makeFiles
  makeRecords
  reverseDate
  cd ..
done
