echo -e -n "\e[?1049h"

sleep 0.01s

let x=0
let y=0

while (($y < $LINES)); do
	let x=0
	while (($x < $COLUMNS)); do
		let "a=((2*y+0)*255)/(2*LINES)"
		let "b=((2*y+1)*255)/(2*LINES)"
		echo -e -n "\e[48;2;255;0;${a}m\e[38;2;255;0;${b}m\u2584"
		let x++
	done
	let y++
	if (($y < $LINES)); then
		echo -e -n "\e[m\n"
	fi
done

echo -e -n "\e[H"

read -s -n 1
echo -e -n "\e[?1049l"
