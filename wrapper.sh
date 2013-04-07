# example values
CARDS=45
REPETITIONS=11
MODE=pile # or riffle
PILES=5

if ! which convert 2>&1 >/dev/null; then
	echo "Please install imagemagick"
	exit 1
fi

if [ "$MODE" = "pile" ]; then
	FNAME="$CARDS-${PILES}pile.png"
else
	FNAME="$CARDS-$MODE.png"
fi

if [ -f "$FNAME" ]; then
	echo "$FNAME already exists; please remove it"
	exit 1
fi

make
OUTPUTS=
for i in `seq 0 $REPETITIONS`; do
	TMP=`mktemp XXXXXXXX.bmp`
	./shuffler -m $MODE -r $i -n $CARDS -p $PILES $TMP
	OUTPUTS="$OUTPUTS $TMP"
done
convert $OUTPUTS -append $FNAME
rm $OUTPUTS
