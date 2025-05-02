echo "4-bit colours (0x00 - 0x0F)"
for y in {0..1}; do
	for x in {0..7}; do
		echo -e -n "\e[48;5;$(($x + $y*8))m    "
	done
	echo -e "\e[m"
done

echo

echo "8-bit colours (0x10 - 0xE7)"
for y in {0..5}; do
	for i in {0..5}; do
		for x in {0..5}; do
			echo -e -n "\e[48;5;$(($x + $y*6 + $i*36 + 16))m  "
		done
		if ((i < 5)); then
			echo -e -n "\e[m "
		fi
	done
	echo -e "\e[m"
done

echo

echo "8-bit colours greyscale (0xE8 - 0xFF)"
for y in {0..1}; do
	for x in {0..23}; do
		echo -e -n "\e[48;5;$(($x + 232))m   "
	done
	echo -e "\e[m"
done

echo -e "\nSee more on \e]8;;https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters\e\\Wikipedia\e]8;;\e\\"
