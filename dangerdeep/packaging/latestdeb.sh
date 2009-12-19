#!/bin/bash
# Dont forget to set the environment variables SOURCE, DEBFULLNAME, EMAIL and KEY_ID (todo: add check)
CACHE_DIR="/tmp/cache/dailydeb/"

. $1

svn up -q $SOURCE

OLDIFS=$IFS
IFS=":"
MAXREVNO=0
UPDATED=false
for i in ${ADD}; do
	i="${SOURCE}${i}"
	if [ -r "${CACHE_DIR}${i}/${PACKAGE}.old" ]; then
		OLDREVNO=`cat ${CACHE_DIR}${i}/${PACKAGE}.old`
	else
		OLDREVNO=0
		mkdir -p ${CACHE_DIR}${i}
	fi

	REVNO=`LC_ALL=C svn info ${i} | grep 'Revision:' | awk '{print $2}'`

	if [ $REVNO -gt $OLDREVNO ]; then
		UPDATED=true
		echo $REVNO > ${CACHE_DIR}${i}/${PACKAGE}.old
	fi
	if [ $REVNO -gt $MAXREVNO ]; then
		MAXREVNO=$REVNO
	fi

done

if ! $UPDATED; then
	exit
fi

REVNO=$MAXREVNO

DATE=`date +%Y%m%d`
WORKDIR="/tmp/mkdeb-$RANDOM/"
VERSION="svn${DATE}r${REVNO}"
PKGDIR=${WORKDIR}${PACKAGE}-${VERSION}/

mkdir $WORKDIR
mkdir $PKGDIR


for i in ${ADD}; do
	cp -r ${SOURCE}$i $PKGDIR
done

for i in $REMOVE; do
	rm -rf $PKGDIR$i
done
IFS=$OLDIFS

svn2cl --break-before-msg $SOURCE -o ${PKGDIR}ChangeLog -r $REVNO
dch -v $VERSION --create --package $PACKAGE -D $DIST -c ${PKGDIR}debian/changelog "Initial release"

cd $PKGDIR
dpkg-buildpackage -k${KEY_ID} -S $DPKG_ARGS

dput $PUT ${WORKDIR}${PACKAGE}_${VERSION}_source.changes 

rm -rf $WORKDIR
