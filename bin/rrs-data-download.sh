#!/bin/sh

set -u -e

RSYNC="rsync -rtzv --progress --stats "
REPO="www.ryzom.com::ryzom/data"
DEST="data/"

SRC=""
FILES="characters_*.bnp fauna_*.bnp construction.bnp data_common.bnp leveldesign.bnp objects.bnp sfx.bnp"
FILES="${FILES} race_stats.packed_sheets item.packed_sheets sitem.packed_sheets"

for f in ${FILES}; do
	SRC="${SRC} ${REPO}/${f}"
done

$RSYNC $SRC $DEST

