#! /bin/sh
# Run Inno Setup Compiler to create installers for magnetic datasets

INNO="c:/Program Files/Inno Setup 5/ISCC.exe"
test -f "$INNO" || INNO="c:/Program Files (x86)/Inno Setup 5/ISCC.exe"

MAGNETICDIR=..
(
cat <<EOF
emm2017 EMM2017
EOF
) | while read prefix name; do
    "$INNO" magnetic-installers.iss \
	//dMAGNETICDIR="$MAGNETICDIR" \
	//dPREFIX="$prefix" \
	//dNAME="$name" > $prefix.log 2>&1
done
exit 0

cat <<EOF
wmm2010 WMM2010
emm2010 EMM2010
igrf11  IGRF11
wmm2015 WMM2015
igrf12  IGRF12
emm2015 EMM2015
emm2017 EMM2017
EOF
