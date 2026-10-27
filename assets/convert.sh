for file in *.svg; do
  rsvg-convert -w 256 -h 256 --keep-aspect-ratio "$file" -o "${file%.svg}.png"
done