# convert icons
for FROM in icons/*.jpg
do
  TO=`echo $FROM | sed 's/icons\//assets\//' | cut -f1 -d\.`.jpg
  convert $FROM -bordercolor White -border 20x20 -scale 100x100 -colorspace sRGB -type truecolor -sampling-factor 2x2,1x1,1x1 -interlace none $TO
done