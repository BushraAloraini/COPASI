#!/bin/bash 


TESTSDIR=./dsmts24
#TESTSDIR=./dsmts23-20080307

HEAD=/usr/bin/head
CUT=/usr/bin/cut
SED=//usr/bin/sed

#HEAD=/sw/bin/head
#CUT=/sw/bin/cut
#SED=/sw/bin/sed

#WRAPPER=./stochastic-testsuite

WRAPPER=`pwd`/stochastic-testsuite.app/Contents/MacOS/stochastic-testsuite

# go over all models in tests/model-list
#MODELS=`cat tests/model-list`

if [ $# != 2 ] ; then
  echo -e "\nUsage: run.sh MODEL_LIST_FILE NUM_REPEATS\n";
  exit 1;
fi  

MODELFILE=$1
NUM_REPEATS=$2

MODELS=`cat $MODELFILE` 


function getSpecies()
{
    $HEAD -n1 $1 | $CUT -d ',' -f 2- | $SED -e ' s/,/ /g; s/[ \t\r\n]*$//';
}

TOTALTIME=0.0


for MODEL in $MODELS;do
  # set parameters 
  RESULT="passed"
  ENDTIME=50
  STEPNUMBER=50
  INFILE=${TESTSDIR}/${MODEL}.xml
  OUTFILE=${TESTSDIR}/${MODEL}.RESULT
  MEAN_OUTFILE=${TESTSDIR}/${MODEL}-mean.RESULT
  SD_OUTFILE=${TESTSDIR}/${MODEL}-sd.RESULT
  SPECIESLIST=`getSpecies ${TESTSDIR}/${MODEL}-mean.csv`
  MEAN_REFERENCE_FILE=${TESTSDIR}/${MODEL}-mean.csv
  SD_REFERENCE_FILE=${TESTSDIR}/${MODEL}-sd.csv
  # run simulation 
  if [ -e $INFILE ] ; then
    echo "$WRAPPER $INFILE $ENDTIME $STEPNUMBER $NUM_REPEATS $OUTFILE $SPECIESLIST"
    rm -f $OUTFILE
    exec 3>&1 4>&2
    TIME=$( { time -p $WRAPPER $INFILE $ENDTIME $STEPNUMBER $NUM_REPEATS $OUTFILE $SPECIESLIST || RESULT="failed"  1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    TIME=`echo $TIME |  /usr/bin/awk '{print $4}'` 
    echo "time: $TIME seconds"
    TOTALTIME=`echo "$TIME + $TOTALTIME" | /usr/bin/bc` 
    if [ "e$RESULT" == "efailed" ] ; then
      echo "ERROR: Simulation failed.";
      rm -f $OUTFILE;
    fi  
    if [ -e $OUTFILE ] ; then
      # calculate mean and standard deviation
      rm -f $MEAN_OUTFILE $SD_OUTFILE
      ./calculate_statistics.py $OUTFILE $MEAN_OUTFILE $SD_OUTFILE $STEPNUMBER $NUM_REPEATS || RESULT="failed" ;
      if [ "e$RESULT" == "efailed" ] ; then
        echo "ERROR: Statistical calculation failed.";
        rm -f $MEAN_OUTFILE $SD_OUTFILE;
      fi  
      # compare results
      if [ -e ${MEAN_OUTFILE} ] ; then
        if [ -e ${SD_REFERENCE_FILE} ] ; then 
          if [ -e ${MEAN_REFERENCE_FILE} ] ; then
             rm -f ${TESTSDIR}/${MODEL}-mean-compare.RESULT 
             ./compare_mean.py ${MEAN_OUTFILE} ${MEAN_REFERENCE_FILE} ${SD_REFERENCE_FILE} $NUM_REPEATS ${TESTSDIR}/${MODEL}-mean-compare.RESULT || RESULT="failed"; 
             rm -f ${TESTSDIR}/${MODEL}-sd-compare.RESULT 
             ./compare_sd.py ${SD_OUTFILE} ${TESTSDIR}/${MODEL}-sd.csv $NUM_REPEATS ${TESTSDIR}/${MODEL}-sd-compare.RESULT || RESULT="failed";
          else
            echo "Error. Could not find file \"${MEAN_REFERENCE_FILE}\"";
            RESULT="failed"
          fi     
        else
          echo "Error. Could not find file \"${SD_REFERENCE_FILE}\"";
          RESULT="failed"
        fi
      else
        echo "Error. Could not find file \"${MEAN_OUTFILE}\"";
        RESULT="failed"
      fi  
    else
      echo "Error. Could not find file \"${OUTFILE}\"";
      RESULT="failed"
    fi
  else
    echo "Error. Could not  find file \"${INFILE}\"";
    RESULT="failed"
  fi
  echo "$MODEL $RESULT"
done

echo "Time used for simulations: $TOTALTIME seconds";
