./configure || (echo ARRRRGGGGG && exit 1)�
make || (echo ARRRRGGGGGorf && exit 1)�

if [ -c /dev/sound/dsp ]
then
  src/sisy
else
  src/sisy -a /dev/dsp
fi
