./configure || (echo ARRRRGGGGG && exit 1)Ã
make || (echo ARRRRGGGGGorf && exit 1)Ã

if [ -c /dev/sound/dsp ]
then
  src/sisy
else
  src/sisy -a /dev/dsp
fi
