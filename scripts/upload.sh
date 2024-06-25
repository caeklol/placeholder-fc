PICO_PATH=''
FOUND_PICO=0

build_bootsel() {
	cc scripts/bootsel.c -o scripts/bootsel $(pkg-config --cflags --libs libusb-1.0)
}

find_pico() {
	scripts/bootsel

	if [[ $OSTYPE == 'darwin'* ]]; then
		disks=$(ls -1 /Volumes | awk '{print "/Volumes/"$1}')
	else
		disks=$(cat /proc/mounts | awk '{print $2}')
	fi

	for disk in $disks
	do
		info_uf2="$disk/INFO_UF2.TXT"
		if [ -f $info_uf2 ]
		then
			echo "Found mounted Pico on $disk!"
			PICO_PATH=$disk
			FOUND_PICO=1
		fi
	done
}

if [ ! -f build/code.uf2 ]; then
	echo "Could not find UF2 file! Have you run ./build.sh?"
	exit 1
fi

if [ ! -f scripts/bootsel ]; then
	 build_bootsel
fi

echo "Waiting for Pico..."

while : ; do
	find_pico

	if [ "$FOUND_PICO" -ne 0 ]; then
		break
	fi

	sleep 0.1
done

mv build/code.uf2 $PICO_PATH
echo "Successfully uploaded UF2 to Pico!"
