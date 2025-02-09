for INPUTFILE in ../instances/*.stp
do
	INPUTFILE=${INPUTFILE##*/}
	echo "./runSingle.sh $INPUTFILE"
	./runSingle.sh $INPUTFILE
done

